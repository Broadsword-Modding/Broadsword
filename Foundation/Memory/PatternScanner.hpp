#pragma once

#include <Windows.h>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace Broadsword::Foundation {

/**
 * Pattern scanning utility for finding code patterns in memory.
 *
 * Used for finding functions/data in games without debug symbols.
 * Supports wildcards (? for any byte).
 *
 * Pattern formats:
 *   - IDA style: "48 8B 05 ? ? ? ? 48 85 C0"
 *   - Code style: "\x48\x8B\x05\x00\x00\x00\x00\x48\x85\xC0" with mask "xxx????xxx"
 *
 * Usage:
 *   uintptr_t address = PatternScanner::Scan("HalfSwordUE5-Win64-Shipping.exe",
 *                                             "48 8B 05 ? ? ? ? 48 85 C0");
 */
class PatternScanner {
public:
  /**
   * Scan for a pattern in a specific module.
   *
   * @param moduleName Name of the module to scan (e.g., "game.exe")
   * @param pattern IDA-style pattern with wildcards
   * @return Address of first match, or 0 if not found
   */
  static uintptr_t Scan(const char* moduleName, const char* pattern);

  /**
   * Scan for a pattern in a specific memory region.
   *
   * @param startAddress Start of region to scan
   * @param size Size of region in bytes
   * @param pattern IDA-style pattern with wildcards
   * @return Address of first match, or 0 if not found
   */
  static uintptr_t ScanRegion(uintptr_t startAddress, size_t size, const char* pattern);

  /**
   * Scan for a pattern using raw bytes and mask.
   *
   * @param startAddress Start of region to scan
   * @param size Size of region in bytes
   * @param bytes Raw pattern bytes
   * @param mask Mask string ('x' = match, '?' = wildcard)
   * @return Address of first match, or 0 if not found
   */
  static uintptr_t ScanWithMask(uintptr_t startAddress, size_t size, const char* bytes, const char* mask);

  /**
   * Find all matches of a pattern in a module.
   *
   * @param moduleName Name of the module to scan
   * @param pattern IDA-style pattern with wildcards
   * @param maxResults Maximum number of results (0 = unlimited)
   * @return Vector of addresses
   */
  static std::vector<uintptr_t> ScanAll(const char* moduleName, const char* pattern, size_t maxResults = 0);

  /**
   * Scan for a pattern relative to an address.
   * Useful for RIP-relative addressing in x64.
   *
   * @param moduleName Module to scan
   * @param pattern IDA-style pattern
   * @param offsetToRelative Offset from pattern match to the relative offset
   * @param instructionLength Length of the instruction (for RIP calculation)
   * @return Resolved absolute address, or 0 if not found
   */
  static uintptr_t ScanRelative(const char* moduleName, const char* pattern, size_t offsetToRelative,
                                size_t instructionLength);

  /**
   * Parse an IDA-style pattern into bytes and mask.
   *
   * @param pattern IDA-style pattern (e.g., "48 8B 05 ? ? ? ?")
   * @param outBytes Output vector for pattern bytes
   * @param outMask Output string for mask
   * @return true on success, false on invalid pattern
   */
  static bool ParsePattern(const char* pattern, std::vector<uint8_t>& outBytes, std::string& outMask);

  /**
   * Get module information (base + size).
   *
   * @param moduleName Name of the module
   * @param outBase Output for module base address
   * @param outSize Output for module size
   * @return true if module found, false otherwise
   */
  static bool GetModuleInfo(const char* moduleName, uintptr_t& outBase, size_t& outSize);

private:
  // Internal scan implementation
  static uintptr_t ScanInternal(uintptr_t start, size_t size, const uint8_t* bytes, const char* mask, size_t length);
};

} // namespace Broadsword::Foundation
