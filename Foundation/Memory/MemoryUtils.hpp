#pragma once

#include <Windows.h>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace Broadsword::Foundation {

/**
 * Memory manipulation utilities.
 *
 * Provides safe wrappers for common memory operations:
 * - Read/write with protection changes
 * - Memory region queries
 * - Module enumeration
 * - Address validation
 */
class MemoryUtils {
public:
  /**
   * Read memory safely (handles protection).
   *
   * @tparam T Type to read
   * @param address Address to read from
   * @param outValue Reference to store the value
   * @return true on success, false on failure
   */
  template <typename T>
  static bool Read(uintptr_t address, T& outValue) {
    if (!IsValidReadAddress(reinterpret_cast<void*>(address), sizeof(T))) {
      return false;
    }

    outValue = *reinterpret_cast<T*>(address);
    return true;
  }

  /**
   * Write memory safely (handles protection).
   *
   * @tparam T Type to write
   * @param address Address to write to
   * @param value Value to write
   * @return true on success, false on failure
   */
  template <typename T>
  static bool Write(uintptr_t address, const T& value) {
    DWORD oldProtect;
    if (!VirtualProtect(reinterpret_cast<void*>(address), sizeof(T), PAGE_READWRITE, &oldProtect)) {
      return false;
    }

    *reinterpret_cast<T*>(address) = value;

    VirtualProtect(reinterpret_cast<void*>(address), sizeof(T), oldProtect, &oldProtect);
    return true;
  }

  /**
   * Read a pointer and follow it.
   *
   * @param address Address of the pointer
   * @return Dereferenced pointer, or 0 on failure
   */
  static uintptr_t ReadPointer(uintptr_t address);

  /**
   * Follow a pointer chain (multi-level pointer).
   *
   * @param baseAddress Starting address
   * @param offsets Vector of offsets to follow
   * @return Final address, or 0 on failure
   */
  static uintptr_t FollowPointerChain(uintptr_t baseAddress, const std::vector<uintptr_t>& offsets);

  /**
   * Check if an address is valid for reading.
   *
   * @param address Address to check
   * @param size Size of the region to check
   * @return true if readable, false otherwise
   */
  static bool IsValidReadAddress(void* address, size_t size = 1);

  /**
   * Check if an address is valid for writing.
   *
   * @param address Address to check
   * @param size Size of the region to check
   * @return true if writable, false otherwise
   */
  static bool IsValidWriteAddress(void* address, size_t size = 1);

  /**
   * Check if an address is valid for execution.
   *
   * @param address Address to check
   * @return true if executable, false otherwise
   */
  static bool IsValidExecuteAddress(void* address);

  /**
   * Get module base address by name.
   *
   * @param moduleName Name of the module (e.g., "ntdll.dll")
   * @return Base address, or 0 if not found
   */
  static uintptr_t GetModuleBase(const char* moduleName);

  /**
   * Get module size.
   *
   * @param moduleBase Base address of the module
   * @return Size in bytes, or 0 on failure
   */
  static size_t GetModuleSize(uintptr_t moduleBase);

  /**
   * Get all loaded modules in the current process.
   *
   * @return Vector of module base addresses
   */
  static std::vector<uintptr_t> GetLoadedModules();

  /**
   * Change memory protection.
   *
   * @param address Address to change
   * @param size Size of the region
   * @param newProtection New protection flags (PAGE_*)
   * @param outOldProtection Optional: receives old protection
   * @return true on success, false on failure
   */
  static bool ChangeProtection(void* address, size_t size, DWORD newProtection, DWORD* outOldProtection = nullptr);

  /**
   * Allocate executable memory.
   *
   * @param size Size to allocate
   * @return Pointer to allocated memory, or nullptr on failure
   */
  static void* AllocateExecutable(size_t size);

  /**
   * Free memory allocated with AllocateExecutable.
   *
   * @param address Pointer to free
   */
  static void FreeExecutable(void* address);

  /**
   * Calculate relative address from absolute (for x64 RIP-relative addressing).
   *
   * @param from Source address (where the instruction is)
   * @param to Destination address (where we want to jump)
   * @param instructionLength Length of the instruction (usually 5 for JMP)
   * @return 32-bit relative offset
   */
  static int32_t CalculateRelativeAddress(uintptr_t from, uintptr_t to, size_t instructionLength);

  /**
   * Calculate absolute address from relative (for x64 RIP-relative addressing).
   *
   * @param instructionAddress Address of the instruction
   * @param relativeOffset Relative offset from the instruction
   * @param instructionLength Length of the instruction
   * @return Absolute address
   */
  static uintptr_t CalculateAbsoluteAddress(uintptr_t instructionAddress, int32_t relativeOffset,
                                            size_t instructionLength);
};

} // namespace Broadsword::Foundation
