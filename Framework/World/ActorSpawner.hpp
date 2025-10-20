#pragma once

#include "../../ModAPI/Result.hpp"
#include "../../Engine/SDK/SDK.hpp"

namespace Broadsword {

/**
 * ActorSpawner - Type-safe actor spawning
 *
 * Provides safe, type-checked spawning of UE5 actors with proper
 * error handling and parameter validation.
 *
 * Features:
 * - Template-based type safety
 * - Result monad for error handling
 * - Location and rotation support
 * - Spawn parameter customization
 *
 * Thread Safety:
 * - MUST be called from game thread only
 * - Uses UE5 SDK which is NOT thread-safe
 *
 * Usage:
 *   auto spawner = ActorSpawner(world);
 *
 *   // Spawn at location
 *   auto result = spawner.Spawn<SDK::AWillie_BP_C>(
 *       SDK::AWillie_BP_C::StaticClass(),
 *       {0.0f, 0.0f, 100.0f},  // location
 *       {0.0f, 0.0f, 0.0f}     // rotation
 *   );
 *
 *   if (result) {
 *       SDK::AWillie_BP_C* actor = result.Value();
 *       // Use spawned actor
 *   }
 */
class ActorSpawner {
public:
    /**
     * Construct spawner with world context
     */
    explicit ActorSpawner(SDK::UWorld* world) : m_World(world) {}

    /**
     * Spawn an actor of specific type
     *
     * @tparam T Actor type to spawn
     * @param actorClass UClass for the actor
     * @param location Spawn location in world space
     * @param rotation Spawn rotation (pitch, yaw, roll)
     * @return Result<T*> - Spawned actor or error
     */
    template<typename T>
    Result<T*> Spawn(
        SDK::UClass* actorClass,
        const SDK::FVector& location,
        const SDK::FRotator& rotation = {0.0f, 0.0f, 0.0f}
    ) {
        if (!m_World) {
            return Error::WorldNotLoaded;
        }

        if (!actorClass) {
            return Error::InvalidActorClass;
        }

        // Create transform
        SDK::FTransform transform;
        transform.Translation = location;
        transform.Rotation = SDK::FQuat(rotation);
        transform.Scale3D = {1.0f, 1.0f, 1.0f};

        // Begin deferred spawn
        SDK::AActor* spawnedActor = SDK::UGameplayStatics::BeginDeferredActorSpawnFromClass(
            m_World,
            actorClass,
            transform,
            SDK::ESpawnActorCollisionHandlingMethod::AlwaysSpawn,
            nullptr,
            SDK::ESpawnActorScaleMethod::MultiplyWithRoot
        );

        if (!spawnedActor) {
            return Error::SpawnFailed;
        }

        // Finish spawning
        SDK::UGameplayStatics::FinishSpawningActor(
            spawnedActor,
            transform,
            SDK::ESpawnActorScaleMethod::MultiplyWithRoot
        );

        // Cast to requested type
        T* typedActor = static_cast<T*>(spawnedActor);
        if (!typedActor) {
            return Error::InvalidActorClass;
        }

        return typedActor;
    }

    /**
     * Spawn actor at specific transform
     *
     * @tparam T Actor type
     * @param actorClass UClass for the actor
     * @param transform Complete transform (location, rotation, scale)
     * @return Result<T*> - Spawned actor or error
     */
    template<typename T>
    Result<T*> SpawnAtTransform(
        SDK::UClass* actorClass,
        const SDK::FTransform& transform
    ) {
        if (!m_World) {
            return Error::WorldNotLoaded;
        }

        if (!actorClass) {
            return Error::InvalidActorClass;
        }

        // Begin deferred spawn
        SDK::AActor* spawnedActor = SDK::UGameplayStatics::BeginDeferredActorSpawnFromClass(
            m_World,
            actorClass,
            transform,
            SDK::ESpawnActorCollisionHandlingMethod::AlwaysSpawn,
            nullptr,
            SDK::ESpawnActorScaleMethod::MultiplyWithRoot
        );

        if (!spawnedActor) {
            return Error::SpawnFailed;
        }

        // Finish spawning
        SDK::UGameplayStatics::FinishSpawningActor(
            spawnedActor,
            transform,
            SDK::ESpawnActorScaleMethod::MultiplyWithRoot
        );

        T* typedActor = static_cast<T*>(spawnedActor);
        if (!typedActor) {
            return Error::InvalidActorClass;
        }

        return typedActor;
    }

private:
    SDK::UWorld* m_World;
};

} // namespace Broadsword
