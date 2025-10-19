#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <optional>
#include <filesystem>
#include <source_location>

namespace Broadsword {

/**
 * BindingID - Stable identifier for keybindings
 *
 * Uses std::source_location to generate unique IDs based on call site.
 * This ensures the same button always gets the same binding, even across sessions.
 *
 * Format: "ModName::FileName::Line::Column::Label"
 * Example: "EnhancerMod::EnhancerMod.cpp::145::12::God Mode"
 *
 * Hash collision resistance:
 * - File path uniquely identifies source file
 * - Line/column uniquely identifies call site
 * - Label provides human-readable context
 * - Modulo hash for efficient lookup
 */
struct BindingID {
    std::string modName;
    std::string fileName;
    uint32_t line;
    uint32_t column;
    std::string label;

    /**
     * Generate stable hash for this binding
     * Used as key in binding storage
     */
    size_t Hash() const;

    /**
     * Get human-readable string representation
     * Format: "ModName::Label (File:Line)"
     */
    std::string ToString() const;

    /**
     * Create BindingID from source location
     *
     * @param modName Name of the mod registering this binding
     * @param label Human-readable label for the binding
     * @param loc Source location (automatically captured at call site)
     */
    static BindingID FromLocation(
        std::string_view modName,
        std::string_view label,
        std::source_location loc = std::source_location::current()
    );

    // Comparison operators for use in maps
    bool operator==(const BindingID& other) const;
    bool operator!=(const BindingID& other) const;
};

/**
 * BindingManager - Universal keybinding system
 *
 * Allows any UI widget to be bound to a keyboard key, with automatic
 * persistence across sessions. Uses std::source_location for stable IDs.
 *
 * Features:
 * - Automatic registration on first use
 * - Persistent storage in JSON
 * - Key conflict detection
 * - Rebinding UI support
 * - Per-mod namespacing
 *
 * Thread Safety:
 * - All methods are game thread only
 * - No synchronization needed (single-threaded)
 *
 * Usage:
 *   // In mod OnRegister:
 *   ctx.bindings.Register("MyMod", "Toggle Feature");
 *
 *   // In mod OnFrame (via UIContext):
 *   if (ctx.ui.Button("Toggle Feature")) {
 *       // Button clicked OR bound key pressed
 *   }
 *
 *   // Manual binding check:
 *   if (ctx.bindings.WasKeyPressed("MyMod", "Toggle Feature")) {
 *       // Bound key was pressed this frame
 *   }
 */
class BindingManager {
public:
    BindingManager() = default;
    ~BindingManager() = default;

    /**
     * Register a binding
     *
     * Must be called before using WasKeyPressed or GetKey.
     * Typically called once during mod initialization.
     *
     * @param id Binding identifier
     * @param displayLabel Human-readable label for UI
     */
    void Register(const BindingID& id, std::string_view displayLabel);

    /**
     * Bind a key to an action
     *
     * @param id Binding identifier
     * @param vkey Virtual key code (VK_* constants from Windows)
     */
    void Bind(const BindingID& id, int vkey);

    /**
     * Unbind a key from an action
     *
     * @param id Binding identifier
     */
    void Unbind(const BindingID& id);

    /**
     * Check if the bound key was pressed this frame
     *
     * @param id Binding identifier
     * @return true if bound key was pressed this frame, false otherwise
     */
    bool WasKeyPressed(const BindingID& id);

    /**
     * Get the currently bound key
     *
     * @param id Binding identifier
     * @return Virtual key code if bound, std::nullopt otherwise
     */
    std::optional<int> GetKey(const BindingID& id) const;

    /**
     * Get display label for a binding
     *
     * @param id Binding identifier
     * @return Display label, or empty string if not registered
     */
    std::string GetDisplayLabel(const BindingID& id) const;

    /**
     * Check if a binding is registered
     *
     * @param id Binding identifier
     * @return true if registered, false otherwise
     */
    bool IsRegistered(const BindingID& id) const;

    /**
     * Update key states (called once per frame by framework)
     *
     * Polls keyboard state and updates internal pressed state.
     * Must be called before any WasKeyPressed checks.
     */
    void Update();

    /**
     * Save bindings to JSON file
     *
     * @param path Path to JSON file
     * @return true if save succeeded, false otherwise
     */
    bool Save(const std::filesystem::path& path) const;

    /**
     * Load bindings from JSON file
     *
     * @param path Path to JSON file
     * @return true if load succeeded, false otherwise
     */
    bool Load(const std::filesystem::path& path);

    /**
     * Get all registered bindings
     *
     * @return Map of binding ID hash to display label
     */
    std::unordered_map<size_t, std::string> GetAllBindings() const;

private:
    /**
     * Binding data for a single action
     */
    struct Binding {
        BindingID id;
        std::string displayLabel;
        std::optional<int> vkey;
        bool wasPressed = false;      // Was pressed last frame
        bool isPressedNow = false;    // Is pressed this frame
    };

    /**
     * Get Windows key state
     *
     * @param vkey Virtual key code
     * @return true if key is currently down, false otherwise
     */
    bool IsKeyDown(int vkey) const;

    // Storage: Hash -> Binding
    std::unordered_map<size_t, Binding> m_Bindings;
};

} // namespace Broadsword

// Hash function for BindingID to use in unordered_map
namespace std {
    template<>
    struct hash<Broadsword::BindingID> {
        size_t operator()(const Broadsword::BindingID& id) const {
            return id.Hash();
        }
    };
}
