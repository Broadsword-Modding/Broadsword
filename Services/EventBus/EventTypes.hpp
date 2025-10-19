#pragma once

#include "../../ModAPI/Frame.hpp"

namespace Broadsword {

/**
 * Event emitted every frame
 *
 * Mods subscribe to this event to receive per-frame updates.
 * All mod logic should be triggered by events, not polled manually.
 *
 * Usage:
 *   void OnRegister(ModContext& ctx) override {
 *       ctx.events.Subscribe<OnFrameEvent>([this](OnFrameEvent& e) {
 *           OnFrame(e.frame);
 *       });
 *   }
 *
 *   void OnFrame(Frame& frame) {
 *       // Per-frame logic here
 *   }
 */
struct OnFrameEvent {
    Frame& frame;
    float deltaTime;
};

/**
 * Event emitted when a UWorld is loaded
 * To be implemented later
 */
struct OnWorldLoadedEvent {
    // SDK::UWorld* world;
};

/**
 * Event emitted when a UWorld is unloaded
 * To be implemented later
 */
struct OnWorldUnloadedEvent {
    // SDK::UWorld* world;
};

/**
 * Event emitted when the player is spawned
 * To be implemented later
 */
struct OnPlayerSpawnedEvent {
    // SDK::AWillie_BP_C* player;
};

/**
 * Event emitted when the player dies
 * To be implemented later
 */
struct OnPlayerDiedEvent {
    // SDK::AWillie_BP_C* player;
    // std::string deathReason;
};

/**
 * Event emitted when an actor is spawned
 * To be implemented later
 */
struct OnActorSpawnedEvent {
    // SDK::AActor* actor;
    // SDK::UClass* actorClass;
};

} // namespace Broadsword
