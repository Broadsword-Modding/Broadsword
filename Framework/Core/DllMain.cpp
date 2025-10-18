#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <fstream>
#include <memory>
#include <string>

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

// Logging (temporary for Phase 1)
static std::ofstream g_LogFile;

static void Log(const std::string& message)
{
    if (g_LogFile.is_open())
    {
        g_LogFile << message << std::endl;
        g_LogFile.flush();
    }
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

        // Initialize DX11 render backend (Phase 1: DX11 only)
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

        // Render test window for Phase 1
        {
            ImGui::Begin("Broadsword Framework - Phase 1");
            ImGui::Text("Framework is running!");
            ImGui::Text("Backend: DirectX 11");
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
            ImGui::Text("Queued Actions: %zu", GameThreadExecutor::Get().PendingCount());
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Phase 1 Complete!");
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

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        DisableThreadLibraryCalls(hModule);

        // Open log file
        g_LogFile.open("Broadsword_Phase1.log", std::ios::out | std::ios::trunc);
        Log("Broadsword Framework - Phase 1");
        Log("DLL_PROCESS_ATTACH - Broadsword.dll loaded");

        // Initialize kiero for DX11 (Phase 1: DX11 only)
        // Kiero will wait for DirectX to be initialized by the game
        Log("Initializing kiero (DX11 mode)...");
        if (!VTableHook::Initialize(VTableHook::RenderAPI::DX11))
        {
            Log("Failed to initialize kiero!");
            return TRUE; // Don't fail DLL load, continue anyway
        }

        Log("kiero initialized successfully");

        // Hook Present (index 8)
        // This will be called once DirectX renders a frame
        Log("Hooking IDXGISwapChain::Present (index 8)...");
        if (!VTableHook::Bind<PresentFn>(8, (PresentFn**)&oPresent, hkPresent))
        {
            Log("Failed to hook Present!");
            return TRUE;
        }

        Log("Present hooked successfully");

        // Hook ResizeBuffers (index 13) for window resize handling
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
