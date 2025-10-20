#pragma once

#include "../../ModAPI/Config.hpp"
#include <type_traits>
#include <string>
#include <concepts>

namespace Broadsword {

/**
 * Type trait helpers for config reflection
 *
 * These will be replaced with proper C++26 reflection when available.
 * For now, we use compile-time type checking and manual registration.
 */

// Concept for serializable types
template<typename T>
concept Serializable =
    std::is_same_v<T, bool> ||
    std::is_same_v<T, int> ||
    std::is_same_v<T, float> ||
    std::is_same_v<T, double> ||
    std::is_same_v<T, std::string> ||
    std::is_enum_v<T>;

/**
 * Field descriptor for manual config registration
 *
 * Until C++26 reflection is available, mods must manually register
 * their config fields using this descriptor.
 *
 * Usage:
 *   struct MyConfig {
 *       [[configurable]] bool godMode = false;
 *       [[configurable]] float speed = 1.0f;
 *
 *       static auto GetConfigFields() {
 *           return std::array{
 *               FieldDescriptor{"godMode", offsetof(MyConfig, godMode), FieldType::Bool},
 *               FieldDescriptor{"speed", offsetof(MyConfig, speed), FieldType::Float},
 *           };
 *       }
 *   };
 */
enum class FieldType {
    Bool,
    Int,
    Float,
    Double,
    String,
    Enum
};

struct FieldDescriptor {
    const char* name;
    size_t offset;
    FieldType type;
};

/**
 * Future C++26 reflection example (not yet implemented)
 *
 * When C++26 static reflection is available, this will be the approach:
 *
 * template<typename T>
 * auto GetConfigFields() {
 *     std::vector<FieldDescriptor> fields;
 *
 *     for... (auto member : members_of(^T)) {
 *         if constexpr (has_attribute(member, configurable)) {
 *             fields.push_back({
 *                 .name = identifier_of(member),
 *                 .offset = offset_of(member),
 *                 .type = DeduceFieldType<type_of(member)>()
 *             });
 *         }
 *     }
 *
 *     return fields;
 * }
 */

} // namespace Broadsword
