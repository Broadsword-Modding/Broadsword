#pragma once

#include <Windows.h>
#include <string>
#include <vector>

namespace Broadsword::Foundation {

/**
 * DLL Hijacking utility for loading a framework DLL via proxy DLL.
 *
 * Pattern: Game loads our dwmapi.dll (proxy) → Proxy loads real dwmapi.dll from System32
 *          and forwards all exports → Proxy also loads Broadsword.dll (framework)
 *
 * This allows injection without modifying game files or using external injectors.
 */
class DllHijack {
public:
  /**
   * Load the real system DLL and return its handle.
   * Searches System32 directory to avoid recursion.
   *
   * @param dllName Name of the DLL to load (e.g., "dwmapi.dll")
   * @return Handle to the real system DLL, or nullptr on failure
   */
  static HMODULE LoadRealSystemDll(const char* dllName);

  /**
   * Get a function pointer from the real system DLL.
   *
   * @param hModule Handle to the real system DLL
   * @param functionName Name of the function to get
   * @return Function pointer, or nullptr if not found
   */
  static FARPROC GetRealFunction(HMODULE hModule, const char* functionName);

  /**
   * Load the framework DLL (Broadsword.dll) from the same directory as the proxy.
   *
   * @param proxyModule Handle to the proxy DLL (us)
   * @return Handle to the framework DLL, or nullptr on failure
   */
  static HMODULE LoadFrameworkDll(HMODULE proxyModule);

  /**
   * Get the directory path of a loaded module.
   *
   * @param hModule Handle to the module
   * @return Directory path (without trailing backslash), or empty string on failure
   */
  static std::string GetModuleDirectory(HMODULE hModule);

  /**
   * Check if a file exists.
   *
   * @param path Path to the file
   * @return true if file exists, false otherwise
   */
  static bool FileExists(const std::string& path);

  /**
   * Get all export names from a DLL.
   * Useful for generating .def files or logging available functions.
   *
   * @param hModule Handle to the module
   * @return Vector of export names
   */
  static std::vector<std::string> GetExportNames(HMODULE hModule);

  /**
   * Create a forwarding stub for a function.
   * Not used in practice (we use .def files), but kept for reference.
   *
   * @param realFunction Pointer to the real function
   * @return Pointer to the forwarding stub
   */
  static void* CreateForwardingStub(FARPROC realFunction);
};

} // namespace Broadsword::Foundation
