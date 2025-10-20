#include "WorldFacade.hpp"

namespace Broadsword {

Result<SDK::UWorld*> WorldFacade::GetWorld() const {
    // Try cached world first
    if (m_CachedWorld) {
        return m_CachedWorld;
    }

    // Fallback: Query from SDK
    SDK::UWorld* world = SDK::UWorld::GetWorld();
    if (!world) {
        return Error::WorldNotLoaded;
    }

    // Update cache
    m_CachedWorld = world;
    return world;
}

Result<SDK::AWillie_BP_C*> WorldFacade::GetPlayer() const {
    // Get world first
    auto worldResult = GetWorld();
    if (!worldResult) {
        return worldResult.GetError();
    }
    SDK::UWorld* world = worldResult.Value();

    // Get player controller
    SDK::APlayerController* controller = SDK::UGameplayStatics::GetPlayerController(world, 0);
    if (!controller) {
        return Error::PlayerNotFound;
    }

    // Get pawn
    SDK::APawn* pawn = controller->K2_GetPawn();
    if (!pawn) {
        return Error::PlayerNotSpawned;
    }

    // Cast to Willie (player character)
    SDK::AWillie_BP_C* player = static_cast<SDK::AWillie_BP_C*>(pawn);
    if (!player) {
        return Error::PlayerNotSpawned;
    }

    return player;
}

Result<SDK::AWorldSettings*> WorldFacade::GetWorldSettings() const {
    // Get world first
    auto worldResult = GetWorld();
    if (!worldResult) {
        return worldResult.GetError();
    }
    SDK::UWorld* world = worldResult.Value();

    // Access PersistentLevel to get world settings
    if (!world->PersistentLevel) {
        return Error::WorldInvalid;
    }

    SDK::AWorldSettings* settings = world->PersistentLevel->WorldSettings;
    if (!settings) {
        return Error::WorldInvalid;
    }

    return settings;
}

bool WorldFacade::IsWorldLoaded() const {
    auto result = GetWorld();
    return result.IsOk();
}

bool WorldFacade::IsPlayerSpawned() const {
    auto result = GetPlayer();
    return result.IsOk();
}

void WorldFacade::UpdateWorldPointer() {
    SDK::UWorld* newWorld = SDK::UWorld::GetWorld();

    if (newWorld != m_CachedWorld) {
        m_CachedWorld = newWorld;
    }
}

} // namespace Broadsword
