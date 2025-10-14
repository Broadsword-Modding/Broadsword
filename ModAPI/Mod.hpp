#pragma once

#include "Frame.hpp"
#include <nlohmann/json.hpp>
#include <string>

namespace Broadsword {

/**
 * Mod metadata.
 */
struct ModInfo {
  std::string name;
  std::string version;
  std::string author;
  std::string description;
};

/**
 * Base class for all Broadsword mods.
 *
 * Mods implement ONE callback: OnFrame(Frame&)
 * This runs every frame on the game thread after the game has rendered.
 *
 * Inside OnFrame, you can:
 * - Access UE5 SDK directly (safe - we're on game thread)
 * - Render ImGui UI (safe - we're in the render pass)
 * - Handle input
 * - Log messages
 * - Subscribe to events
 * - Hook functions
 *
 * No separate OnUpdate/OnRenderUI/OnInitialize callbacks!
 * Everything happens in OnFrame (or in event callbacks you register).
 *
 * Configuration:
 * - Mark fields with [[configurable]] attribute (C++26 reflection)
 * - Framework auto-serializes/deserializes to Broadsword.config.json
 * - OnSaveConfig/OnLoadConfig for manual control if needed
 *
 * Example:
 *   class MyMod : public Broadsword::Mod {
 *     [[configurable]] bool m_GodMode = false;
 *     [[configurable]] float m_Speed = 1.0f;
 *
 *     ModInfo GetInfo() const override {
 *       return {"My Mod", "1.0.0", "Author", "Description"};
 *     }
 *
 *     void OnFrame(Frame& frame) override {
 *       // Game logic + UI in one place!
 *       if (frame.ui.Button("Toggle God Mode")) {
 *         m_GodMode = !m_GodMode;
 *       }
 *
 *       if (m_GodMode) {
 *         auto* player = frame.world.GetPlayer();
 *         player->Health = 100.0f;
 *       }
 *     }
 *   };
 */
class Mod {
public:
  virtual ~Mod() = default;

  /**
   * Get mod metadata.
   * This is required for all mods.
   */
  virtual ModInfo GetInfo() const = 0;

  /**
   * Called every frame on the game thread.
   *
   * This is THE callback where everything happens:
   * - Game logic
   * - UI rendering
   * - Input handling
   * - Everything!
   *
   * @param frame Frame context with access to all systems
   */
  virtual void OnFrame(Frame& frame) {}

  /**
   * Save mod configuration to JSON.
   * Override this for manual config control.
   * If not overridden, framework uses [[configurable]] reflection.
   *
   * @return JSON object with config data
   */
  virtual nlohmann::json OnSaveConfig() { return {}; }

  /**
   * Load mod configuration from JSON.
   * Override this for manual config control.
   * If not overridden, framework uses [[configurable]] reflection.
   *
   * @param config JSON object with config data
   */
  virtual void OnLoadConfig(const nlohmann::json& config) {}

  /**
   * Check if mod is enabled.
   * Framework only calls OnFrame() if enabled.
   */
  bool IsEnabled() const { return m_enabled; }

  /**
   * Set mod enabled state.
   */
  void SetEnabled(bool enabled) { m_enabled = enabled; }

private:
  bool m_enabled = true;
};

} // namespace Broadsword

/**
 * Export macro for mods.
 * Every mod DLL must use this to export CreateMod and DestroyMod functions.
 *
 * Usage:
 *   class MyMod : public Broadsword::Mod { ... };
 *   BROADSWORD_EXPORT_MOD(MyMod)
 */
#define BROADSWORD_EXPORT_MOD(ModClass)                                                                                \
  extern "C" {                                                                                                         \
  __declspec(dllexport) Broadsword::Mod* CreateMod() { return new ModClass(); }                                       \
  __declspec(dllexport) void DestroyMod(Broadsword::Mod* mod) { delete mod; }                                         \
  }
