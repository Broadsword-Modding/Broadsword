#pragma once

#include <cstdint>

namespace Broadsword {

// Forward declarations
class WorldFacade;
class UIContext;
class InputContext;
class Logger;

/**
 * Frame context passed to mods every frame via OnFrameEvent
 *
 * Provides dependency injection for all framework services that mods need
 * to interact with the game world, UI, input, and logging.
 *
 * All references are guaranteed to be valid for the duration of the frame.
 *
 * Usage:
 *   void OnFrame(Frame& frame) {
 *       auto player = frame.world.GetPlayer();
 *       if (frame.ui.Button("Heal")) { ... }
 *       frame.log.Info("Frame {}", frame.frameNumber);
 *   }
 */
struct Frame {
    /**
     * High-level facade for world operations
     * - GetWorld(), GetPlayer(), GetWorldSettings()
     * - Spawn<T>(), FindActor<T>(), etc.
     */
    WorldFacade& world;

    /**
     * UI context for rendering ImGui elements
     * - Button(), Checkbox(), Slider(), etc.
     * - All UI elements support universal keybinding
     */
    UIContext& ui;

    /**
     * Input context for querying input state
     * - IsKeyPressed(), IsKeyDown(), GetMousePosition(), etc.
     */
    InputContext& input;

    /**
     * Structured logger with frame tracking
     * - Info(), Warn(), Error(), Debug()
     * - Automatically includes frame number and source location
     */
    Logger& log;

    /**
     * Time elapsed since last frame in seconds
     * Typically 0.016 for 60 FPS (16ms)
     */
    float deltaTime;

    /**
     * Current frame number since framework started
     * Useful for logging, debugging, and temporal logic
     */
    uint64_t frameNumber;
};

} // namespace Broadsword
