#pragma once

#include <filesystem>

namespace Broadsword {

/**
 * UniversalConfig - Stub for config system
 *
 * Manages framework and mod configuration.
 * Will be fully implemented in future checkpoints.
 */
class UniversalConfig {
public:
    UniversalConfig() = default;

    /**
     * Save config to file (stub)
     */
    bool Save(const std::filesystem::path& path) {
        return true; // Stub
    }

    /**
     * Load config from file (stub)
     */
    bool Load(const std::filesystem::path& path) {
        return true; // Stub
    }
};

} // namespace Broadsword
