#include "DllHijack.hpp"
#include <algorithm>
#include <filesystem>

namespace Broadsword::Foundation {

HMODULE DllHijack::LoadRealSystemDll(const char* dllName) {
  // Get System32 directory
  char systemPath[MAX_PATH];
  if (GetSystemDirectoryA(systemPath, MAX_PATH) == 0) {
    return nullptr;
  }

  // Build full path to real DLL
  std::string fullPath = std::string(systemPath) + "\\" + dllName;

  // Load the real system DLL
  HMODULE hModule = LoadLibraryA(fullPath.c_str());
  return hModule;
}

FARPROC DllHijack::GetRealFunction(HMODULE hModule, const char* functionName) {
  if (!hModule || !functionName) {
    return nullptr;
  }

  return GetProcAddress(hModule, functionName);
}

HMODULE DllHijack::LoadFrameworkDll(HMODULE proxyModule) {
  // Get directory where proxy DLL is located
  std::string proxyDir = GetModuleDirectory(proxyModule);
  if (proxyDir.empty()) {
    return nullptr;
  }

  // Build path to Broadsword.dll
  std::string frameworkPath = proxyDir + "\\Broadsword.dll";

  // Check if framework DLL exists
  if (!FileExists(frameworkPath)) {
    // Try alternate location (for development builds)
    frameworkPath = proxyDir + "\\..\\Broadsword.dll";
    if (!FileExists(frameworkPath)) {
      return nullptr;
    }
  }

  // Load framework DLL
  HMODULE hFramework = LoadLibraryA(frameworkPath.c_str());
  return hFramework;
}

std::string DllHijack::GetModuleDirectory(HMODULE hModule) {
  char modulePath[MAX_PATH];
  if (GetModuleFileNameA(hModule, modulePath, MAX_PATH) == 0) {
    return "";
  }

  // Extract directory from full path
  std::string path(modulePath);
  size_t lastSlash = path.find_last_of("\\/");
  if (lastSlash != std::string::npos) {
    return path.substr(0, lastSlash);
  }

  return "";
}

bool DllHijack::FileExists(const std::string& path) {
  DWORD attributes = GetFileAttributesA(path.c_str());
  return (attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY));
}

std::vector<std::string> DllHijack::GetExportNames(HMODULE hModule) {
  std::vector<std::string> exports;

  if (!hModule) {
    return exports;
  }

  // Get DOS header
  PIMAGE_DOS_HEADER dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(hModule);
  if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
    return exports;
  }

  // Get NT headers
  PIMAGE_NT_HEADERS ntHeaders =
      reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<BYTE*>(hModule) + dosHeader->e_lfanew);
  if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) {
    return exports;
  }

  // Get export directory
  IMAGE_DATA_DIRECTORY exportDir = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
  if (exportDir.VirtualAddress == 0) {
    return exports;
  }

  PIMAGE_EXPORT_DIRECTORY exportDirectory =
      reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(reinterpret_cast<BYTE*>(hModule) + exportDir.VirtualAddress);

  // Get export name table
  DWORD* nameRVAs = reinterpret_cast<DWORD*>(reinterpret_cast<BYTE*>(hModule) + exportDirectory->AddressOfNames);

  // Iterate through all exported names
  for (DWORD i = 0; i < exportDirectory->NumberOfNames; i++) {
    const char* name = reinterpret_cast<const char*>(reinterpret_cast<BYTE*>(hModule) + nameRVAs[i]);
    exports.push_back(name);
  }

  return exports;
}

void* DllHijack::CreateForwardingStub(FARPROC realFunction) {
  // This is a simplified example - in practice, we use .def files for forwarding
  // Kept here for reference/documentation purposes

  // Allocate executable memory for the stub
  void* stub = VirtualAlloc(nullptr, 16, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
  if (!stub) {
    return nullptr;
  }

  // x64 stub: movabs rax, address; jmp rax
  unsigned char* code = static_cast<unsigned char*>(stub);
  code[0] = 0x48;  // REX.W prefix
  code[1] = 0xB8;  // MOV RAX, imm64
  *reinterpret_cast<uint64_t*>(code + 2) = reinterpret_cast<uint64_t>(realFunction);
  code[10] = 0xFF;  // JMP RAX
  code[11] = 0xE0;

  return stub;
}

} // namespace Broadsword::Foundation
