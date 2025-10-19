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
#include "../../Services/UI/UIContext.hpp"
#include "../UI/ConsoleWindow.hpp"
#include "../UI/NotificationManager.hpp"
#include "../UI/ModMenuUI.hpp"
#include <nlohmann/json.hpp>

using namespace Broadsword;
using namespace Broadsword::Foundation;
using namespace Broadsword::Services;
using namespace Broadsword::Framework;

// Global state
static std::unique_ptr<RenderBackend> g_RenderBackend = nullptr;
static std::unique_ptr<ConsoleWindow> g_ConsoleWindow = nullptr;
static std::unique_ptr<ModMenuUI> g_ModMenuUI = nullptr;
static bool g_Initialized = false;
static bool g_ShuttingDown = false;
static HWND g_Window = nullptr;

// Logging
static std::ofstream g_LogFile;

static void Log(const std::string& message)
{
    // Write to file
    if (g_LogFile.is_open())
    {
        g_LogFile << message << std::endl;
        g_LogFile.flush();
    }

    // Also add to in-game console if initialized
    if (g_ConsoleWindow)
    {
        g_ConsoleWindow->AddMessage(LogLevel::Info, message);
    }
}

// Forward declare Win32 message handler
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Original WndProc
static WNDPROC oWndProc = nullptr;

// Custom WndProc to handle input
static LRESULT CALLBACK hkWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Handle INSERT key to toggle UI visibility
    if (msg == WM_KEYDOWN && wParam == VK_INSERT)
    {
        if (g_ModMenuUI)
        {
            g_ModMenuUI->ToggleVisible();
        }
        return 0;
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
        Log("First Present call detected - initializing Broadsword Framework...");

        // Get window handle
        DXGI_SWAP_CHAIN_DESC desc;
        pSwapChain->GetDesc(&desc);
        g_Window = desc.OutputWindow;
        Log("Got window handle");

        // Initialize ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        Log("ImGui context created");

        // Initialize ImGui Win32 backend
        ImGui_ImplWin32_Init(g_Window);
        Log("ImGui Win32 backend initialized");

        // Hook WndProc for input handling
        oWndProc = (WNDPROC)SetWindowLongPtrA(g_Window, GWLP_WNDPROC, (LONG_PTR)hkWndProc);
        if (oWndProc)
        {
            Log("WndProc hooked successfully");
        }
        else
        {
            Log("Failed to hook WndProc!");
        }

        // Initialize DX11 render backend
        Log("Creating DX11 render backend...");
        g_RenderBackend = CreateRenderBackend(RenderBackend::API::DX11);

        if (g_RenderBackend && g_RenderBackend->InitializeImGui(pSwapChain))
        {
            Log("DX11 backend initialized successfully");

            // Initialize UI system
            Log("Initializing UI system...");
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
                    Log("Loaded config from Broadsword.json");
                }
                catch (const std::exception& e)
                {
                    Log(std::string("Failed to parse config: ") + e.what());
                }
            }
            else
            {
                Log("No config file found, using defaults");
            }

            // Apply theme to ImGui
            UIContext::Get().GetTheme().ApplyToImGui();
            Log("Theme applied to ImGui");

            // Create UI windows
            g_ConsoleWindow = std::make_unique<ConsoleWindow>();
            g_ModMenuUI = std::make_unique<ModMenuUI>();
            g_ModMenuUI->SetConsoleWindow(g_ConsoleWindow.get());

            // Load console config after creation
            std::ifstream consoleConfigFile(configPath);
            if (consoleConfigFile.is_open())
            {
                try
                {
                    nlohmann::json config;
                    consoleConfigFile >> config;
                    consoleConfigFile.close();

                    g_ConsoleWindow->LoadFromConfig(config);
                    Log("Loaded console settings from config");
                }
                catch (const std::exception& e)
                {
                    Log(std::string("Failed to load console config: ") + e.what());
                }
            }

            Log("UI windows created");

            g_Initialized = true;
        }
        else
        {
            Log("Failed to initialize DX11 backend!");
            g_RenderBackend.reset();
            g_Initialized = false;
        }

        if (g_Initialized)
        {
            Log("Broadsword Framework initialized successfully!");

            // Show welcome notification
            NotificationManager::Get().Success("Broadsword Framework", "Framework initialized successfully!");
        }
        else
        {
            Log("Broadsword Framework initialization FAILED!");
        }
    }

    if (g_Initialized && g_RenderBackend)
    {
        // Process game thread actions
        GameThreadExecutor::Get().ProcessQueue();

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
    Log("Broadsword initialization thread started");

    // Retry loop with timeout
    const int MAX_RETRIES = 100;      // Maximum attempts
    const int RETRY_DELAY_MS = 100;   // 100ms between retries
    const int TIMEOUT_MS = 10000;     // 10 second total timeout

    int attempt = 0;
    bool kieroInitialized = false;

    Log("Waiting for DirectX to be initialized by the game...");

    while (attempt < MAX_RETRIES && !kieroInitialized)
    {
        attempt++;

        // Try to initialize kiero
        if (VTableHook::Initialize(VTableHook::RenderAPI::DX11))
        {
            kieroInitialized = true;
            Log("kiero initialized successfully on attempt " + std::to_string(attempt));
        }
        else
        {
            // DirectX not ready yet, wait and retry
            if (attempt % 10 == 0) // Log every 10 attempts to avoid spam
            {
                Log("kiero init attempt " + std::to_string(attempt) + " failed, retrying...");
            }
            Sleep(RETRY_DELAY_MS);
        }
    }

    if (!kieroInitialized)
    {
        Log("CRITICAL ERROR: kiero failed to initialize after " + std::to_string(attempt) + " attempts (" +
            std::to_string(TIMEOUT_MS / 1000) + " seconds)");
        Log("DirectX may not be D3D11, or game is using incompatible graphics API");
        return 1;
    }

    // Hook Present (index 8)
    Log("Hooking IDXGISwapChain::Present (index 8)...");
    if (!VTableHook::Bind<PresentFn>(8, (PresentFn**)&oPresent, hkPresent))
    {
        Log("Failed to hook Present!");
        return 1;
    }
    Log("Present hooked successfully");

    // Hook ResizeBuffers (index 13)
    Log("Hooking IDXGISwapChain::ResizeBuffers (index 13)...");
    if (!VTableHook::Bind<ResizeBuffersFn>(13, (ResizeBuffersFn**)&oResizeBuffers, hkResizeBuffers))
    {
        Log("Failed to hook ResizeBuffers!");
    }
    else
    {
        Log("ResizeBuffers hooked successfully");
    }

    Log("Hooks installed - waiting for first Present call to complete initialization...");

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        DisableThreadLibraryCalls(hModule);

        // Open log file with timestamp
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::tm localTime;
        localtime_s(&localTime, &time);

        char logFilename[512];
        sprintf_s(logFilename, "Broadsword_%04d%02d%02d_%02d%02d%02d.log",
                  localTime.tm_year + 1900, localTime.tm_mon + 1,
                  localTime.tm_mday, localTime.tm_hour, localTime.tm_min,
                  localTime.tm_sec);

        g_LogFile.open(logFilename, std::ios::out | std::ios::trunc);
        Log("Broadsword Framework");
        Log("DLL_PROCESS_ATTACH - Broadsword.dll loaded");

        // Create initialization thread (don't block DLL_PROCESS_ATTACH)
        HANDLE hThread = CreateThread(NULL, 0, BroadswordThread, NULL, 0, NULL);
        if (hThread)
        {
            CloseHandle(hThread); // We don't need to wait for it
            Log("Initialization thread created");
        }
        else
        {
            Log("CRITICAL ERROR: Failed to create initialization thread!");
        }

        break;
    }

    case DLL_PROCESS_DETACH:
    {
        Log("DLL_PROCESS_DETACH");
        g_ShuttingDown = true;

        // Small delay to ensure no hooks are executing
        Sleep(100);

        // Save config before cleanup
        try
        {
            nlohmann::json config;
            UIContext::Get().GetTheme().SaveToConfig(config);

            if (g_ConsoleWindow)
            {
                g_ConsoleWindow->SaveToConfig(config);
            }

            std::ofstream configFile("Broadsword.json");
            if (configFile.is_open())
            {
                configFile << config.dump(2);
                configFile.close();
                Log("Saved config to Broadsword.json");
            }
        }
        catch (const std::exception& e)
        {
            Log(std::string("Failed to save config: ") + e.what());
        }

        // Cleanup UI system
        g_ModMenuUI.reset();
        g_ConsoleWindow.reset();
        NotificationManager::Get().Clear();
        UIContext::Get().Shutdown();

        // Restore original WndProc
        if (g_Window && oWndProc)
        {
            SetWindowLongPtrA(g_Window, GWLP_WNDPROC, (LONG_PTR)oWndProc);
        }

        // Cleanup render backend
        if (g_RenderBackend)
        {
            g_RenderBackend->ShutdownImGui();
            g_RenderBackend.reset();
        }

        if (ImGui::GetCurrentContext())
        {
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
        }

        VTableHook::Shutdown();

        Log("Broadsword Framework shut down");
        g_LogFile.close();
        break;
    }
    }

    return TRUE;
}
