#pragma once

/**
 * Broadsword Framework - Main API Header
 *
 * Include this single header in your mod to access the entire Broadsword API.
 *
 * Quick start:
 *   #include <Broadsword.hpp>
 *
 *   class MyMod : public Broadsword::Mod {
 *     [[configurable]] bool m_Enabled = true;
 *
 *     ModInfo GetInfo() const override {
 *       return {"My Mod", "1.0.0", "Author", "Description"};
 *     }
 *
 *     void OnFrame(Frame& frame) override {
 *       // Everything happens here - game logic + UI!
 *       if (frame.ui.Button("Do Thing")) {
 *         // Direct SDK access - no queuing!
 *         auto* player = frame.world.GetPlayer();
 *         player->Health = 100.0f;
 *       }
 *     }
 *   };
 *
 *   BROADSWORD_EXPORT_MOD(MyMod)
 */

// Version
#define BROADSWORD_VERSION_MAJOR 2
#define BROADSWORD_VERSION_MINOR 0
#define BROADSWORD_VERSION_PATCH 0

// Core API
#include "Mod.hpp"
#include "Frame.hpp"

// Systems (will be added in Phase 2+)
// #include "World.hpp"      // World access
// #include "UI.hpp"         // ImGui wrappers
// #include "Input.hpp"      // Input handling
// #include "Hooks.hpp"      // ProcessEvent hooking
// #include "Utils.hpp"      // Utilities

// For now, Phase 1 provides:
// - Mod base class
// - Frame structure (metadata only)
// - Export macro

namespace Broadsword {

/**
 * Get framework version string.
 */
inline const char* GetVersion() {
  return "2.0.0";
}

/**
 * Check if we're on the game thread.
 * In Broadsword, this should always be true from mod code.
 */
bool IsGameThread();

} // namespace Broadsword
