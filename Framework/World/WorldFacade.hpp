#pragma once

#include "../../ModAPI/Result.hpp"
#include "../../Engine/SDK/SDK.hpp"

namespace Broadsword {

/**
 * WorldFacade - High-level facade for world operations
 *
 * Provides type-safe access to UE5 world state with proper error handling.
 * All methods return Result<T, Error> for explicit error handling.
 *
 * Thread Safety:
 * - MUST be called from game thread only
 * - UE5 SDK is NOT thread-safe
 * - Use GameThread::QueueAction() from render thread
 *
 * Usage:
 *   void OnFrame(Frame& frame) {
 *       auto worldResult = frame.world.GetWorld();
 *       if (!worldResult) {
 *           log.Error("World not loaded: {}", ToString(worldResult.GetError()));
 *           return;
 *       }
 *       SDK::UWorld* world = worldResult.Value();
 *       // Use world safely
 *   }
 */
class WorldFacade {
public:
    WorldFacade() = default;
    ~WorldFacade() = default;

    /**
     * Get the current UWorld instance
     *
     * @return Result<UWorld*> - World pointer or Error::WorldNotLoaded
     */
    Result<SDK::UWorld*> GetWorld() const;

    /**
     * Get the player character (Willie)
     *
     * @return Result<AWillie_BP_C*> - Player pointer or error
     * @error Error::WorldNotLoaded if world is null
     * @error Error::PlayerNotFound if player controller is null
     * @error Error::PlayerNotSpawned if pawn is null or wrong type
     */
    Result<SDK::AWillie_BP_C*> GetPlayer() const;

    /**
     * Get world settings
     *
     * @return Result<AWorldSettings*> - World settings pointer or error
     * @error Error::WorldNotLoaded if world is null
     * @error Error::WorldInvalid if world settings is null
     */
    Result<SDK::AWorldSettings*> GetWorldSettings() const;

    /**
     * Check if world is currently loaded and valid
     *
     * @return true if world is loaded and accessible
     */
    bool IsWorldLoaded() const;

    /**
     * Check if player is spawned and valid
     *
     * @return true if player exists and is accessible
     */
    bool IsPlayerSpawned() const;

private:
    /**
     * Cached world pointer (updated each frame by framework)
     * This is set by the framework main loop to avoid repeated lookups
     */
    mutable SDK::UWorld* m_CachedWorld = nullptr;

    friend class FrameworkCore; // Allows framework to update cache
};

} // namespace Broadsword
