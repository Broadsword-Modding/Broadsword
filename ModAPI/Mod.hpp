#pragma once

#include "ModInfo.hpp"

namespace Broadsword {

// Forward declarations
struct ModContext;

/**
 * Base interface for all Broadsword mods
 *
 * Lifecycle:
 * 1. CreateMod() called by ModLoader
 * 2. OnRegister() called once when mod is loaded
 * 3. OnUnregister() called once when mod is unloaded or framework shuts down
 * 4. DestroyMod() called by ModLoader
 *
 * Mods receive frame updates by subscribing to OnFrameEvent in OnRegister:
 *   ctx.events.Subscribe<OnFrameEvent>([this](auto& e) { OnFrame(e.frame); });
 */
class Mod {
public:
    virtual ~Mod() = default;

    /**
     * Called once when the mod is loaded
     * Use this to:
     * - Subscribe to events
     * - Register hooks
     * - Initialize state
     *
     * @param ctx Mod registration context with access to events, config, logging, hooks
     */
    virtual void OnRegister(ModContext& ctx) = 0;

    /**
     * Called once when the mod is unloaded or framework shuts down
     * Use this to:
     * - Unsubscribe from events (optional, automatic cleanup)
     * - Unhook ProcessEvent hooks
     * - Clean up resources
     */
    virtual void OnUnregister() = 0;

    /**
     * Get mod metadata
     * This is used for display in the mod menu and logging
     *
     * @return ModInfo struct with Name, Version, Author, Description
     */
    virtual ModInfo GetInfo() const = 0;
};

} // namespace Broadsword

/**
 * Required DLL exports for mod loading
 *
 * Every mod DLL must export these two functions:
 */
extern "C" {
    /**
     * Create an instance of your mod
     * Called by ModLoader during mod discovery
     *
     * @return Pointer to heap-allocated mod instance
     */
    __declspec(dllexport) Broadsword::Mod* CreateMod();

    /**
     * Destroy mod instance
     * Called by ModLoader during shutdown
     *
     * @param mod Pointer to mod instance to delete
     */
    __declspec(dllexport) void DestroyMod(Broadsword::Mod* mod);
}
