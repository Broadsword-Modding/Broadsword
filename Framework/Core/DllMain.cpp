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

using namespace Broadsword;
using namespace Broadsword::Foundation;

// Global state
static std::unique_ptr<RenderBackend> g_RenderBackend = nullptr;
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

#ifdef _DEBUG
    // Write to console
    printf("[Broadsword] %s\n", message.c_str());
#endif
}

// Forward declare Win32 message handler
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

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

        // Initialize DX11 render backend
        Log("Creating DX11 render backend...");
        g_RenderBackend = CreateRenderBackend(RenderBackend::API::DX11);

        if (g_RenderBackend && g_RenderBackend->InitializeImGui(pSwapChain))
        {
            Log("DX11 backend initialized successfully");
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

        // Render test window
        {
            ImGui::Begin("Broadsword Framework");
            ImGui::Text("Framework is running!");
            ImGui::Text("Backend: DirectX 11");
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
            ImGui::Text("Queued Actions: %zu", GameThreadExecutor::Get().PendingCount());
            ImGui::End();
        }

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

#ifdef _DEBUG
        // Allocate console for debug output (stays open for entire session)
        AllocConsole();
        FILE* fDummy;
        freopen_s(&fDummy, "CONOUT$", "w", stdout);
        freopen_s(&fDummy, "CONOUT$", "w", stderr);
        SetConsoleTitleA("Broadsword Framework - Debug Console");

        printf("Broadsword Framework\n\n");
#endif

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

        // Cleanup
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
