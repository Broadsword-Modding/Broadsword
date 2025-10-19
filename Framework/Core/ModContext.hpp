#pragma once

namespace Broadsword {

// Forward declarations
class EventBus;
class UniversalConfig;
class Logger;
class HookContext;

/**
 * Mod registration context passed to OnRegister()
 *
 * Provides dependency injection for services needed during mod initialization:
 * - Event subscriptions
 * - Configuration management
 * - Logging
 * - ProcessEvent hook registration
 *
 * All references remain valid for the entire lifetime of the mod.
 *
 * Usage:
 *   void OnRegister(ModContext& ctx) {
 *       ctx.log.Info("MyMod initializing");
 *
 *       ctx.events.Subscribe<OnFrameEvent>([this](auto& e) {
 *           OnFrame(e.frame);
 *       });
 *
 *       m_hookId = ctx.hooks.Hook("Function Name", [](auto obj, auto func, auto params) {
 *           return true;
 *       });
 *   }
 */
struct ModContext {
    /**
     * Event bus for subscribing to framework and game events
     * - Subscribe<OnFrameEvent>(callback)
     * - Subscribe<OnPlayerSpawnedEvent>(callback)
     * - Subscribe<OnWorldLoadedEvent>(callback)
     */
    EventBus& events;

    /**
     * Universal configuration system
     * Automatically serializes/deserializes [[configurable]] fields
     */
    UniversalConfig& config;

    /**
     * Structured logger
     * Same as Frame::log but available during OnRegister
     */
    Logger& log;

    /**
     * ProcessEvent hook manager
     * - Hook(functionName, callback)
     * - Unhook(hookId)
     */
    HookContext& hooks;
};

} // namespace Broadsword
