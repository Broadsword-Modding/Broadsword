#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <fstream>
#include <memory>
#include <string>
#include <cstdio>
#include <chrono>
#include <ctime>

#include "../../Foundation/Hooks/VTableHook.hpp"
#include "../../Foundation/Threading/GameThreadExecutor.hpp"
#include "../Graphics/RenderBackend.hpp"
#include "../../Services/Logging/Logger.hpp"
#include "../../Services/UI/UIContext.hpp"
#include "../UI/ConsoleWindow.hpp"
#include "../UI/SettingsWindow.hpp"
#include "../UI/AboutWindow.hpp"
#include "../UI/NotificationManager.hpp"
#include "../UI/ModMenuUI.hpp"
#include "ModLoader.hpp"
#include "../World/WorldFacade.hpp"
#include "../../Services/EventBus/EventBus.hpp"
#include "../../Services/EventBus/EventTypes.hpp"
#include "../../Engine/ProcessEventHook.hpp"
#include "../../Services/Input/InputContext.hpp"
#include <nlohmann/json.hpp>

using namespace Broadsword;
using namespace Broadsword::Foundation;
using namespace Broadsword::Services;
using namespace Broadsword::Framework;

// Global state
static std::unique_ptr<RenderBackend> g_RenderBackend = nullptr;
static std::unique_ptr<ConsoleWindow> g_ConsoleWindow = nullptr;
static std::unique_ptr<SettingsWindow> g_SettingsWindow = nullptr;
static std::unique_ptr<AboutWindow> g_AboutWindow = nullptr;
static std::unique_ptr<ModMenuUI> g_ModMenuUI = nullptr;
static std::unique_ptr<ModLoader> g_ModLoader = nullptr;
static std::unique_ptr<WorldFacade> g_WorldFacade = nullptr;
static std::unique_ptr<EventBus> g_EventBus = nullptr;
static std::unique_ptr<InputContext> g_InputContext = nullptr;
static bool g_Initialized = false;
static bool g_ShuttingDown = false;
static HWND g_Window = nullptr;
static uint64_t g_FrameNumber = 0;
static std::atomic<bool> g_LoggerInitialized = false;

// Forward declare Win32 message handler
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Original WndProc
static WNDPROC oWndProc = nullptr;

// Custom WndProc to handle input
static LRESULT CALLBACK hkWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Handle menu toggle key to toggle UI visibility
    if (msg == WM_KEYDOWN && g_SettingsWindow)
    {
        int menuKey = g_SettingsWindow->GetMenuToggleKey();
        if (static_cast<int>(wParam) == menuKey)
        {
            if (g_ModMenuUI)
            {
                g_ModMenuUI->ToggleVisible();
            }
            return 0;
        }
    }

    // Let ImGui handle input when UI is visible
    if (g_ModMenuUI && g_ModMenuUI->IsVisible())
    {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        {
            return true;
        }
    }

    // Pass to original WndProc
    return CallWindowProcA(oWndProc, hWnd, msg, wParam, lParam);
}

// Original Present function pointer
typedef HRESULT(__stdcall* PresentFn)(IDXGISwapChain*, UINT, UINT);
static PresentFn oPresent = nullptr;

// Original ResizeBuffers function pointer
typedef HRESULT(__stdcall* ResizeBuffersFn)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);
static ResizeBuffersFn oResizeBuffers = nullptr;

// Present hook - This is our main framework loop!
static HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT syncInterval, UINT flags)
{
    if (g_ShuttingDown || !pSwapChain)
    {
        return oPresent(pSwapChain, syncInterval, flags);
    }

    // Lazy initialization on first Present call
    // (DirectX is guaranteed to be initialized by now)
    if (!g_Initialized)
    {
        // Wait for logger to be initialized by BroadswordThread
        int waitCount = 0;
        while (!g_LoggerInitialized && waitCount < 100)
        {
            Sleep(10);
            waitCount++;
        }

        if (g_LoggerInitialized)
        {
            Logger::Get().PushContext("Broadsword", "Initialization");
            LOG_INFO("First Present call detected - initializing Broadsword Framework...");
        }

        // Get window handle
        DXGI_SWAP_CHAIN_DESC desc;
        pSwapChain->GetDesc(&desc);
        g_Window = desc.OutputWindow;
        if (g_LoggerInitialized)
        {
            LOG_DEBUG("Got window handle: 0x{:X}", reinterpret_cast<uintptr_t>(g_Window));
        }

        // Initialize ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        if (g_LoggerInitialized) LOG_INFO("ImGui context created");

        // Initialize ImGui Win32 backend
        ImGui_ImplWin32_Init(g_Window);
        if (g_LoggerInitialized) LOG_INFO("ImGui Win32 backend initialized");

        // Hook WndProc for input handling
        oWndProc = (WNDPROC)SetWindowLongPtrA(g_Window, GWLP_WNDPROC, (LONG_PTR)hkWndProc);
        if (oWndProc)
        {
            if (g_LoggerInitialized) LOG_INFO("WndProc hooked successfully");
        }
        else
        {
            if (g_LoggerInitialized) LOG_ERROR("Failed to hook WndProc!");
        }

        // Initialize DX11 render backend
        if (g_LoggerInitialized) LOG_INFO("Creating DX11 render backend...");
        g_RenderBackend = CreateRenderBackend(RenderBackend::API::DX11);

        if (g_RenderBackend && g_RenderBackend->InitializeImGui(pSwapChain))
        {
            if (g_LoggerInitialized) LOG_INFO("DX11 backend initialized successfully");

            // Initialize UI system
            if (g_LoggerInitialized) LOG_INFO("Initializing UI system...");
            UIContext::Get().Initialize();

            // Load config file if it exists
            const char* configPath = "Broadsword.json";
            std::ifstream configFile(configPath);
            if (configFile.is_open())
            {
                try
                {
                    nlohmann::json config;
                    configFile >> config;
                    configFile.close();

                    UIContext::Get().GetTheme().LoadFromConfig(config);
                    if (g_LoggerInitialized) LOG_INFO("Loaded config from Broadsword.json");
                }
                catch (const std::exception& e)
                {
                    if (g_LoggerInitialized) LOG_ERROR("Failed to parse config: {}", e.what());
                }
            }
            else
            {
                if (g_LoggerInitialized) LOG_INFO("No config file found, using defaults");
            }

            // Apply theme to ImGui
            UIContext::Get().GetTheme().ApplyToImGui();
            if (g_LoggerInitialized) LOG_INFO("Theme applied to ImGui");

            // Create UI windows
            g_ConsoleWindow = std::make_unique<ConsoleWindow>();
            g_SettingsWindow = std::make_unique<SettingsWindow>();
            g_AboutWindow = std::make_unique<AboutWindow>();
            g_ModMenuUI = std::make_unique<ModMenuUI>();
            g_ModMenuUI->SetConsoleWindow(g_ConsoleWindow.get());
            g_ModMenuUI->SetSettingsWindow(g_SettingsWindow.get());
            g_ModMenuUI->SetAboutWindow(g_AboutWindow.get());

            // Load window configs after creation
            std::ifstream windowConfigFile(configPath);
            if (windowConfigFile.is_open())
            {
                try
                {
                    nlohmann::json config;
                    windowConfigFile >> config;
                    windowConfigFile.close();

                    g_ConsoleWindow->LoadFromConfig(config);
                    g_SettingsWindow->LoadFromConfig(config);
                    if (g_LoggerInitialized) LOG_INFO("Loaded window settings from config");
                }
                catch (const std::exception& e)
                {
                    if (g_LoggerInitialized) LOG_ERROR("Failed to load window config: {}", e.what());
                }
            }

            if (g_LoggerInitialized) LOG_INFO("UI windows created");

            // Initialize ProcessEvent hook
            if (g_LoggerInitialized) LOG_INFO("Initializing ProcessEvent hook...");
            ProcessEventHook::Get().Initialize();
            if (g_LoggerInitialized) LOG_INFO("ProcessEvent hook initialized");

            // Initialize EventBus
            if (g_LoggerInitialized) LOG_INFO("Initializing EventBus...");
            g_EventBus = std::make_unique<EventBus>();
            if (g_LoggerInitialized) LOG_INFO("EventBus created");

            // Initialize WorldFacade
            if (g_LoggerInitialized) LOG_INFO("Initializing WorldFacade...");
            g_WorldFacade = std::make_unique<WorldFacade>();
            if (g_LoggerInitialized) LOG_INFO("WorldFacade created");

            // Initialize InputContext
            if (g_LoggerInitialized) LOG_INFO("Initializing InputContext...");
            g_InputContext = std::make_unique<InputContext>();
            if (g_LoggerInitialized) LOG_INFO("InputContext created");

            // Initialize ModLoader
            if (g_LoggerInitialized) LOG_INFO("Initializing ModLoader...");
            g_ModLoader = std::make_unique<ModLoader>();
            if (g_LoggerInitialized) LOG_INFO("ModLoader created");

            // Discover and load mods
            if (g_LoggerInitialized) LOG_INFO("Discovering mods in ./Mods directory...");
            size_t modsDiscovered = g_ModLoader->DiscoverMods("./Mods");
            if (g_LoggerInitialized) LOG_INFO("Discovered {} mods", modsDiscovered);

            // TODO: Register mods (call OnRegister for each)
            // For now, we just load the mods but don't register them yet
            // Full registration will be added once all services are ready
            if (g_LoggerInitialized) LOG_INFO("Mod loading complete (registration deferred)");

            g_Initialized = true;
        }
        else
        {
            if (g_LoggerInitialized) LOG_ERROR("Failed to initialize DX11 backend!");
            g_RenderBackend.reset();
            g_Initialized = false;
        }

        if (g_Initialized)
        {
            if (g_LoggerInitialized)
            {
                LOG_INFO("Broadsword Framework initialized successfully!");
                Logger::Get().PopContext();
            }

            // Show welcome notification
            NotificationManager::Get().Success("Broadsword Framework", "Framework initialized successfully!");
        }
        else
        {
            if (g_LoggerInitialized)
            {
                LOG_CRITICAL("Broadsword Framework initialization FAILED!");
                Logger::Get().PopContext();
            }
        }
    }

    if (g_Initialized && g_RenderBackend)
    {
        // Update frame counter
        g_FrameNumber++;
        Logger::Get().SetCurrentFrame(g_FrameNumber);

        // Process game thread actions
        GameThreadExecutor::Get().ProcessQueue();

        // Update keybindings (poll keyboard state)
        UIContext::Get().UpdateBindings();

        // Emit OnFrameEvent to mods
        if (g_EventBus && g_WorldFacade && g_InputContext) {
            // Calculate delta time (simplified for now)
            static auto lastFrameTime = std::chrono::high_resolution_clock::now();
            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float>(currentTime - lastFrameTime).count();
            lastFrameTime = currentTime;

            // Create Frame context
            Frame frame{
                .world = *g_WorldFacade,
                .ui = UIContext::Get(),
                .input = *g_InputContext,
                .log = Logger::Get(),
                .deltaTime = deltaTime,
                .frameNumber = g_FrameNumber
            };

            // Create and emit OnFrameEvent
            OnFrameEvent frameEvent{frame, deltaTime};
            g_EventBus->Emit(frameEvent);
        }

        // Start ImGui frame
        ImGui_ImplWin32_NewFrame();
        g_RenderBackend->NewFrame();
        ImGui::NewFrame();

        // Render UI windows
        if (g_ModMenuUI)
        {
            g_ModMenuUI->Render();
        }

        if (g_ConsoleWindow)
        {
            g_ConsoleWindow->Render();
        }

        if (g_SettingsWindow)
        {
            g_SettingsWindow->Render();
        }

        if (g_AboutWindow)
        {
            g_AboutWindow->Render();
        }

        // Render notifications
        NotificationManager::Get().Render();

        // Render mod UIs
        UIContext::Get().RenderModUIs();

        // Render ImGui
        ImGui::Render();
        g_RenderBackend->RenderDrawData();
    }

    return oPresent(pSwapChain, syncInterval, flags);
}

// ResizeBuffers hook
static HRESULT __stdcall hkResizeBuffers(IDXGISwapChain* pSwapChain, UINT bufferCount, UINT width, UINT height,
                                          DXGI_FORMAT newFormat, UINT swapChainFlags)
{
    if (g_Initialized && g_RenderBackend)
    {
        g_RenderBackend->OnResize(pSwapChain, width, height);
    }

    return oResizeBuffers(pSwapChain, bufferCount, width, height, newFormat, swapChainFlags);
}

// Initialization thread - waits for DirectX to be ready, then hooks it
DWORD WINAPI BroadswordThread(LPVOID lpParam)
{
    // Initialize logger early
    Logger::Get().Initialize();
    g_LoggerInitialized = true;
    Logger::Get().PushContext("Broadsword", "Bootstrap");

    LOG_INFO("Broadsword initialization thread started");

    // Retry loop with timeout
    const int MAX_RETRIES = 100;      // Maximum attempts
    const int RETRY_DELAY_MS = 100;   // 100ms between retries
    const int TIMEOUT_MS = 10000;     // 10 second total timeout

    int attempt = 0;
    bool kieroInitialized = false;

    LOG_INFO("Waiting for DirectX to be initialized by the game...");

    while (attempt < MAX_RETRIES && !kieroInitialized)
    {
        attempt++;

        // Try to initialize kiero
        if (VTableHook::Initialize(VTableHook::RenderAPI::DX11))
        {
            kieroInitialized = true;
            LOG_INFO("kiero initialized successfully on attempt {}", attempt);
        }
        else
        {
            // DirectX not ready yet, wait and retry
            if (attempt % 10 == 0) // Log every 10 attempts to avoid spam
            {
                LOG_DEBUG("kiero init attempt {} failed, retrying...", attempt);
            }
            Sleep(RETRY_DELAY_MS);
        }
    }

    if (!kieroInitialized)
    {
        LOG_CRITICAL("kiero failed to initialize after {} attempts ({} seconds)", attempt, TIMEOUT_MS / 1000);
        LOG_CRITICAL("DirectX may not be D3D11, or game is using incompatible graphics API");
        Logger::Get().PopContext();
        return 1;
    }

    // Hook Present (index 8)
    LOG_INFO("Hooking IDXGISwapChain::Present (index 8)...");
    if (!VTableHook::Bind<PresentFn>(8, (PresentFn**)&oPresent, hkPresent))
    {
        LOG_ERROR("Failed to hook Present!");
        Logger::Get().PopContext();
        return 1;
    }
    LOG_INFO("Present hooked successfully");

    // Hook ResizeBuffers (index 13)
    LOG_INFO("Hooking IDXGISwapChain::ResizeBuffers (index 13)...");
    if (!VTableHook::Bind<ResizeBuffersFn>(13, (ResizeBuffersFn**)&oResizeBuffers, hkResizeBuffers))
    {
        LOG_ERROR("Failed to hook ResizeBuffers!");
    }
    else
    {
        LOG_INFO("ResizeBuffers hooked successfully");
    }

    LOG_INFO("Hooks installed - waiting for first Present call to complete initialization...");
    Logger::Get().PopContext();

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        DisableThreadLibraryCalls(hModule);

        // Create initialization thread (don't block DLL_PROCESS_ATTACH)
        // Logger will be initialized in the thread
        HANDLE hThread = CreateThread(NULL, 0, BroadswordThread, NULL, 0, NULL);
        if (hThread)
        {
            CloseHandle(hThread); // We don't need to wait for it
        }

        break;
    }

    case DLL_PROCESS_DETACH:
    {
        if (g_LoggerInitialized) LOG_INFO("DLL_PROCESS_DETACH - shutting down");
        g_ShuttingDown = true;

        // Small delay to ensure no hooks are executing
        Sleep(100);

        // Save config before cleanup
        try
        {
            if (g_LoggerInitialized) LOG_INFO("Saving configuration...");
            nlohmann::json config;
            UIContext::Get().GetTheme().SaveToConfig(config);

            if (g_ConsoleWindow)
            {
                g_ConsoleWindow->SaveToConfig(config);
            }

            if (g_SettingsWindow)
            {
                g_SettingsWindow->SaveToConfig(config);
            }

            std::ofstream configFile("Broadsword.json");
            if (configFile.is_open())
            {
                configFile << config.dump(2);
                configFile.close();
                if (g_LoggerInitialized) LOG_INFO("Saved config to Broadsword.json");
            }
        }
        catch (const std::exception& e)
        {
            if (g_LoggerInitialized) LOG_ERROR("Failed to save config: {}", e.what());
        }

        // Cleanup UI system
        if (g_LoggerInitialized) LOG_INFO("Cleaning up UI system...");
        g_ModMenuUI.reset();
        g_ConsoleWindow.reset();
        g_SettingsWindow.reset();
        g_AboutWindow.reset();
        NotificationManager::Get().Clear();
        UIContext::Get().Shutdown();

        // Restore original WndProc
        if (g_Window && oWndProc)
        {
            SetWindowLongPtrA(g_Window, GWLP_WNDPROC, (LONG_PTR)oWndProc);
            if (g_LoggerInitialized) LOG_DEBUG("WndProc restored");
        }

        // Cleanup render backend
        if (g_RenderBackend)
        {
            if (g_LoggerInitialized) LOG_INFO("Shutting down render backend...");
            g_RenderBackend->ShutdownImGui();
            g_RenderBackend.reset();
        }

        if (ImGui::GetCurrentContext())
        {
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
            if (g_LoggerInitialized) LOG_DEBUG("ImGui shut down");
        }

        VTableHook::Shutdown();
        if (g_LoggerInitialized) LOG_DEBUG("VTableHook shut down");

        if (g_LoggerInitialized)
        {
            LOG_INFO("Broadsword Framework shut down successfully");
            // Shut down logger last
            Logger::Get().Shutdown();
        }
        break;
    }
    }

    return TRUE;
}
