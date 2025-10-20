#pragma once

namespace Broadsword {

/**
 * [[configurable]] attribute for C++26 reflection
 *
 * Marks class members as automatically serializable to config files.
 * Used with ConfigManager for automatic JSON serialization.
 *
 * C++26 Note:
 * This is a custom attribute that will be detected via reflection
 * when C++26 static reflection becomes available. For now, it serves
 * as a marker that can be parsed by future tooling.
 *
 * Usage:
 *   class MyModConfig {
 *       [[configurable]] bool m_GodMode = false;
 *       [[configurable]] float m_Speed = 1.0f;
 *       [[configurable]] int m_Health = 100;
 *
 *       // Not serialized (no attribute)
 *       std::string m_InternalState;
 *   };
 *
 * Future C++26 Pattern Matching:
 *   template<typename T>
 *   void SerializeConfig(T& config) {
 *       for... (auto member : members_of(^T)) {
 *           if constexpr (has_attribute(member, configurable)) {
 *               // Serialize this member
 *           }
 *       }
 *   }
 */

// Custom attribute tag type
struct configurable_t {
    constexpr configurable_t() = default;
};

// Global instance for attribute usage
inline constexpr configurable_t configurable{};

} // namespace Broadsword

// C++26 attribute declaration
// This allows [[Broadsword::configurable]] syntax
// When C++26 reflection lands, this can be queried via has_attribute()
#define BROADSWORD_CONFIGURABLE [[maybe_unused]]

/**
 * Alternative macro for explicit marking
 * Use this if [[configurable]] doesn't compile on your compiler
 */
#define CONFIGURABLE BROADSWORD_CONFIGURABLE
