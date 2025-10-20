#pragma once

#include "../../ModAPI/Result.hpp"
#include "../../Engine/SDK/SDK.hpp"
#include "ActorSpawner.hpp"
#include "QueryEngine.hpp"
#include <functional>
#include <vector>

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

    // ========================================
    // Spawning Operations
    // ========================================

    /**
     * Spawn an actor at location
     *
     * @tparam T Actor type
     * @param actorClass UClass of actor to spawn
     * @param location World space location
     * @param rotation World space rotation
     * @return Result<T*> - Spawned actor or error
     */
    template<typename T>
    Result<T*> Spawn(
        SDK::UClass* actorClass,
        const SDK::FVector& location,
        const SDK::FRotator& rotation = {0.0f, 0.0f, 0.0f}
    ) {
        auto worldResult = GetWorld();
        if (!worldResult) {
            return worldResult.GetError();
        }

        ActorSpawner spawner(worldResult.Value());
        return spawner.Spawn<T>(actorClass, location, rotation);
    }

    /**
     * Spawn actor at transform
     *
     * @tparam T Actor type
     * @param actorClass UClass of actor
     * @param transform Complete transform
     * @return Result<T*> - Spawned actor or error
     */
    template<typename T>
    Result<T*> SpawnAtTransform(SDK::UClass* actorClass, const SDK::FTransform& transform) {
        auto worldResult = GetWorld();
        if (!worldResult) {
            return worldResult.GetError();
        }

        ActorSpawner spawner(worldResult.Value());
        return spawner.SpawnAtTransform<T>(actorClass, transform);
    }

    // ========================================
    // Query Operations
    // ========================================

    /**
     * Find first actor of type
     *
     * @tparam T Actor type
     * @return Result<T*> - Found actor or error
     */
    template<typename T>
    Result<T*> FindActor() {
        auto worldResult = GetWorld();
        if (!worldResult) {
            return worldResult.GetError();
        }

        QueryEngine engine(worldResult.Value());
        return engine.FindActor<T>();
    }

    /**
     * Find all actors of type
     *
     * @tparam T Actor type
     * @return Vector of actors (empty if none found)
     */
    template<typename T>
    std::vector<T*> FindAllActors() {
        auto worldResult = GetWorld();
        if (!worldResult) {
            return {};
        }

        QueryEngine engine(worldResult.Value());
        return engine.FindAllActors<T>();
    }

    /**
     * Find actors matching predicate
     *
     * @tparam T Actor type
     * @param predicate Filter function
     * @return Vector of matching actors
     */
    template<typename T>
    std::vector<T*> FindActorsWhere(std::function<bool(T*)> predicate) {
        auto worldResult = GetWorld();
        if (!worldResult) {
            return {};
        }

        QueryEngine engine(worldResult.Value());
        return engine.FindActorsWhere<T>(predicate);
    }

    /**
     * Find actors within radius
     *
     * @tparam T Actor type
     * @param location Center point
     * @param radius Search radius
     * @return Vector of actors within radius
     */
    template<typename T>
    std::vector<T*> FindActorsInRadius(const SDK::FVector& location, float radius) {
        auto worldResult = GetWorld();
        if (!worldResult) {
            return {};
        }

        QueryEngine engine(worldResult.Value());
        return engine.FindActorsInRadius<T>(location, radius);
    }

    /**
     * Count actors of type
     *
     * @tparam T Actor type
     * @return Number of actors found
     */
    template<typename T>
    size_t CountActors() {
        auto worldResult = GetWorld();
        if (!worldResult) {
            return 0;
        }

        QueryEngine engine(worldResult.Value());
        return engine.CountActors<T>();
    }

private:
    /**
     * Cached world pointer (updated each frame by framework)
     * This is set by the framework main loop to avoid repeated lookups
     */
    mutable SDK::UWorld* m_CachedWorld = nullptr;

    friend class FrameworkCore; // Allows framework to update cache
};

} // namespace Broadsword
