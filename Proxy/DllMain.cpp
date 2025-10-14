#include "../Foundation/Injection/DllHijack.hpp"
#include <Windows.h>

using namespace Broadsword::Foundation;

// Global state
static HMODULE g_RealDwmapi = nullptr;
static HMODULE g_Framework = nullptr;
static HMODULE g_ProxyModule = nullptr;

/**
 * Proxy DLL entry point.
 *
 * This DLL hijacks dwmapi.dll to inject the Broadsword framework.
 * When the game loads dwmapi.dll, it loads our proxy instead.
 *
 * Our proxy:
 * 1. Loads the real dwmapi.dll from System32
 * 2. Loads Broadsword.dll (the framework)
 * 3. Forwards all dwmapi function calls to the real DLL (via .def file)
 */
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
  switch (ul_reason_for_call) {
  case DLL_PROCESS_ATTACH:
    // Disable thread attach/detach notifications (optimization)
    DisableThreadLibraryCalls(hModule);

    g_ProxyModule = hModule;

    // Load the real dwmapi.dll from System32
    g_RealDwmapi = DllHijack::LoadRealSystemDll("dwmapi.dll");
    if (!g_RealDwmapi) {
      MessageBoxA(nullptr, "Failed to load real dwmapi.dll from System32.\n\nThe game will likely crash.",
                  "Broadsword Proxy Error", MB_OK | MB_ICONERROR);
      return FALSE;
    }

    // Load the Broadsword framework
    g_Framework = DllHijack::LoadFrameworkDll(hModule);
    if (!g_Framework) {
      MessageBoxA(nullptr,
                  "Failed to load Broadsword.dll.\n\n"
                  "Make sure Broadsword.dll is in the same directory as dwmapi.dll.",
                  "Broadsword Proxy Error", MB_OK | MB_ICONERROR);
      // Don't return FALSE - game can still run with real dwmapi, just no framework
    }

    break;

  case DLL_PROCESS_DETACH:
    // Framework will handle its own cleanup via DllMain
    // We just need to free the real dwmapi.dll
    if (g_RealDwmapi) {
      FreeLibrary(g_RealDwmapi);
      g_RealDwmapi = nullptr;
    }
    break;

  case DLL_THREAD_ATTACH:
  case DLL_THREAD_DETACH:
    // Not used (disabled via DisableThreadLibraryCalls)
    break;
  }

  return TRUE;
}

// Export forwarding stubs
// These are placeholders - the actual forwarding is done via Exports.def
// The linker will use the .def file to create forwarding entries in the export table

extern "C" {

// dwmapi.dll has 139 exports - we forward all of them
// The .def file contains: EXPORTS functionName = dwmapi.functionName
// This creates a forwarding export that the Windows loader handles automatically

// Example exports (the .def file has the complete list):
__declspec(dllexport) void DwmEnableBlurBehindWindow() {}
__declspec(dllexport) void DwmExtendFrameIntoClientArea() {}
__declspec(dllexport) void DwmGetColorizationColor() {}
__declspec(dllexport) void DwmIsCompositionEnabled() {}
__declspec(dllexport) void DwmSetWindowAttribute() {}

// ... 134 more exports (see Exports.def for complete list)

} // extern "C"
