#pragma once

#include "../../ModAPI/Result.hpp"
#include "../../Engine/SDK/SDK.hpp"
#include <vector>
#include <functional>

namespace Broadsword {

/**
 * QueryEngine - Type-safe actor queries
 *
 * Provides safe, filtered searching of actors in the world with
 * proper type checking and error handling.
 *
 * Features:
 * - Template-based type filtering
 * - Predicate-based filtering
 * - Single and multi-result queries
 * - Null safety with Result monad
 *
 * Thread Safety:
 * - MUST be called from game thread only
 * - Uses UE5 SDK which is NOT thread-safe
 *
 * Usage:
 *   auto engine = QueryEngine(world);
 *
 *   // Find first actor of type
 *   auto result = engine.FindActor<SDK::AWillie_BP_C>();
 *   if (result) {
 *       SDK::AWillie_BP_C* player = result.Value();
 *   }
 *
 *   // Find all actors of type
 *   auto actors = engine.FindAllActors<SDK::AStaticMeshActor>();
 *
 *   // Find with predicate
 *   auto enemies = engine.FindActorsWhere<SDK::ACharacter>(
 *       [](SDK::ACharacter* actor) {
 *           return actor->Health > 0.0f;
 *       }
 *   );
 */
class QueryEngine {
public:
    /**
     * Construct query engine with world context
     */
    explicit QueryEngine(SDK::UWorld* world) : m_World(world) {}

    /**
     * Find first actor of specific type
     *
     * @tparam T Actor type to find
     * @return Result<T*> - Found actor or Error::ActorNotFound
     */
    template<typename T>
    Result<T*> FindActor() {
        if (!m_World) {
            return Error::WorldNotLoaded;
        }

        // Get persistent level
        if (!m_World->PersistentLevel) {
            return Error::WorldInvalid;
        }

        // Iterate through actors
        auto& actors = m_World->PersistentLevel->Actors;
        for (int32_t i = 0; i < actors.Num(); i++) {
            SDK::AActor* actor = actors[i];
            if (!actor) continue;

            // Try to cast to requested type
            T* typedActor = dynamic_cast<T*>(actor);
            if (typedActor) {
                return typedActor;
            }
        }

        return Error::ActorNotFound;
    }

    /**
     * Find all actors of specific type
     *
     * @tparam T Actor type to find
     * @return Vector of actors (empty if none found)
     */
    template<typename T>
    std::vector<T*> FindAllActors() {
        std::vector<T*> results;

        if (!m_World || !m_World->PersistentLevel) {
            return results;
        }

        auto& actors = m_World->PersistentLevel->Actors;
        for (int32_t i = 0; i < actors.Num(); i++) {
            SDK::AActor* actor = actors[i];
            if (!actor) continue;

            T* typedActor = dynamic_cast<T*>(actor);
            if (typedActor) {
                results.push_back(typedActor);
            }
        }

        return results;
    }

    /**
     * Find actors matching predicate
     *
     * @tparam T Actor type to search
     * @param predicate Filter function (return true to include)
     * @return Vector of matching actors
     */
    template<typename T>
    std::vector<T*> FindActorsWhere(std::function<bool(T*)> predicate) {
        std::vector<T*> results;

        if (!m_World || !m_World->PersistentLevel) {
            return results;
        }

        auto& actors = m_World->PersistentLevel->Actors;
        for (int32_t i = 0; i < actors.Num(); i++) {
            SDK::AActor* actor = actors[i];
            if (!actor) continue;

            T* typedActor = dynamic_cast<T*>(actor);
            if (typedActor && predicate(typedActor)) {
                results.push_back(typedActor);
            }
        }

        return results;
    }

    /**
     * Find actors within radius of location
     *
     * @tparam T Actor type
     * @param location Center point
     * @param radius Search radius
     * @return Vector of actors within radius
     */
    template<typename T>
    std::vector<T*> FindActorsInRadius(const SDK::FVector& location, float radius) {
        float radiusSquared = radius * radius;

        return FindActorsWhere<T>([&](T* actor) {
            SDK::FVector actorLocation = actor->K2_GetActorLocation();
            float distSquared =
                (actorLocation.X - location.X) * (actorLocation.X - location.X) +
                (actorLocation.Y - location.Y) * (actorLocation.Y - location.Y) +
                (actorLocation.Z - location.Z) * (actorLocation.Z - location.Z);

            return distSquared <= radiusSquared;
        });
    }

    /**
     * Count actors of specific type
     *
     * @tparam T Actor type
     * @return Number of actors found
     */
    template<typename T>
    size_t CountActors() {
        return FindAllActors<T>().size();
    }

private:
    SDK::UWorld* m_World;
};

} // namespace Broadsword
