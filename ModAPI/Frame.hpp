#pragma once

#include <cstdint>

namespace Broadsword {

// Forward declarations (will be implemented in Phase 2+)
class WorldContext;
class UIContext;
class InputManager;
class Logger;
class EventBus;
class HookManager;

/**
 * Frame context passed to every mod's OnFrame() callback.
 *
 * Contains everything a mod needs for both game logic and UI:
 * - World access (UE5 SDK calls)
 * - UI rendering (ImGui)
 * - Input handling (keyboard/mouse)
 * - Logging
 * - Events
 * - Hooks
 *
 * All accessible from the same callback, same thread - no queuing needed!
 *
 * Example usage:
 *   void OnFrame(Frame& frame) {
 *     // Game logic
 *     if (frame.input.IsKeyPressed(VK_F1)) {
 *       auto* player = frame.world.GetPlayer();
 *       player->Health = 100.0f;  // Direct SDK access!
 *     }
 *
 *     // UI (same thread, same callback!)
 *     if (frame.ui.Button("Heal")) {
 *       auto* player = frame.world.GetPlayer();  // Direct access!
 *       player->Health = 100.0f;
 *     }
 *   }
 */
struct Frame {
  // Frame timing
  uint64_t number;   // Frame number since start
  float deltaTime;   // Time since last frame (seconds)

  // Core systems (references to singletons - lightweight)
  // These will be implemented in Phase 2+
  // WorldContext& world;      // UE5 world access (FindActor, Spawn, etc.)
  // UIContext& ui;            // ImGui rendering
  // InputManager& input;      // Keyboard/mouse state
  // Logger& log;              // Structured logging
  // EventBus& events;         // Event subscription
  // HookManager& hooks;       // ProcessEvent hooking

  // For Phase 1, we just have frame metadata
  // Systems will be added as we build them
};

} // namespace Broadsword
