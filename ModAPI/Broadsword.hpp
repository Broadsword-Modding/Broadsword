#pragma once

/**
 * Broadsword Framework - Main Public API Header
 *
 * Single include for all mod development needs.
 * This header provides access to the complete Broadsword mod API.
 *
 * Usage:
 *   #include <Broadsword.hpp>
 *
 *   class MyMod : public Broadsword::IMod {
 *       void OnFrame(Broadsword::Frame& frame) override {
 *           // Use frame.world, frame.ui, frame.input, etc.
 *       }
 *   };
 *
 * Features:
 * - IMod interface for mod lifecycle
 * - Frame context with all services
 * - Result monad for error handling
 * - Config reflection with [[configurable]]
 * - ProcessEvent hooks
 * - World operations (spawn, query)
 * - UI widgets with keybinding
 * - Event system
 *
 * Thread Safety:
 * - All SDK operations MUST be on game thread
 * - Frame callbacks are on game thread
 * - Render callbacks are on render thread (use GameThread::QueueAction for SDK)
 */

// ========================================
// Core Mod API
// ========================================
#include "Mod.hpp"           // IMod interface
#include "Frame.hpp"         // Frame context
#include "ModContext.hpp"    // ModContext for OnRegister
#include "Result.hpp"        // Result<T, Error> monad
#include "Error.hpp"         // Error enum

// ========================================
// Configuration System
// ========================================
#include "Config.hpp"        // [[configurable]] attribute

// ========================================
// ProcessEvent Hooks
// ========================================
#include "HookContext.hpp"   // Type-safe hook registration

// ========================================
// Events
// ========================================
#include "Events.hpp"        // OnFrameEvent and custom events

// ========================================
// Bring all Broadsword types into one namespace
// ========================================
namespace Broadsword {

// Core types are already in Broadsword namespace

// Convenience using declarations for commonly used SDK types
namespace SDK {
    // Re-export commonly used SDK types for convenience
    // (Actual SDK types are in ::SDK namespace)
    using ::SDK::UWorld;
    using ::SDK::AActor;
    using ::SDK::UClass;
    using ::SDK::FVector;
    using ::SDK::FRotator;
    using ::SDK::FTransform;
    using ::SDK::UObject;
    using ::SDK::UFunction;
    using ::SDK::AWillie_BP_C;
    using ::SDK::AWorldSettings;
    using ::SDK::APlayerController;
} // namespace SDK

} // namespace Broadsword

/**
 * Quick Start Guide:
 *
 * 1. Include this header:
 *    #include <Broadsword.hpp>
 *
 * 2. Create your mod class:
 *    class MyMod : public Broadsword::IMod {
 *        [[configurable]] bool m_Enabled = true;
 *        [[configurable]] float m_Speed = 1.0f;
 *
 *        void OnRegister(Broadsword::ModContext& ctx) override {
 *            // Subscribe to events, register hooks
 *        }
 *
 *        void OnFrame(Broadsword::Frame& frame) override {
 *            if (!m_Enabled) return;
 *
 *            // Access world
 *            auto world = frame.world.GetWorld();
 *            if (!world) return;
 *
 *            // Access player
 *            auto player = frame.world.GetPlayer();
 *            if (player) {
 *                // Modify player
 *            }
 *
 *            // Spawn actors
 *            auto spawnResult = frame.world.Spawn<SDK::AActor>(
 *                actorClass,
 *                {0, 0, 100}
 *            );
 *
 *            // Query actors
 *            auto enemies = frame.world.FindAllActors<SDK::AWillie_BP_C>();
 *        }
 *
 *        void OnRenderUI(Broadsword::Frame& frame) override {
 *            if (frame.ui.Button("Click Me")) {
 *                // Button was clicked or keybind pressed
 *            }
 *
 *            frame.ui.Checkbox("Enabled", &m_Enabled);
 *            frame.ui.Slider("Speed", &m_Speed, 0.0f, 2.0f);
 *        }
 *    };
 *
 * 3. Export mod creation functions:
 *    extern "C" __declspec(dllexport) Broadsword::IMod* CreateMod() {
 *        return new MyMod();
 *    }
 *
 *    extern "C" __declspec(dllexport) void DestroyMod(Broadsword::IMod* mod) {
 *        delete mod;
 *    }
 */
