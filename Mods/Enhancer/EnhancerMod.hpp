#pragma once

#include <Broadsword.hpp>
#include <vector>
#include <string>

namespace Enhancer {

/**
 * EnhancerMod - Comprehensive gameplay enhancement mod
 *
 * Features:
 * - Player modifications (health, stamina, speed, etc.)
 * - World controls (time scale, gravity, etc.)
 * - NPC spawning with customization
 * - Item spawning
 * - ProcessEvent hooks for advanced features
 */
class EnhancerMod : public Broadsword::Mod {
public:
    EnhancerMod();
    ~EnhancerMod() override = default;

    // Mod interface
    Broadsword::ModInfo GetInfo() const override {
        return {
            .Name = "Enhancer",
            .Version = "2.0.0",
            .Author = "Broadsword Framework",
            .Description = "Comprehensive gameplay enhancement mod"
        };
    }

    void OnRegister(Broadsword::ModContext& ctx) override;
    void OnUnregister() override;

private:
    // ========================================
    // Player Tab Configuration
    // ========================================
    [[configurable]] bool m_PlayerEnabled = false;
    [[configurable]] bool m_InfiniteHealth = false;
    [[configurable]] bool m_InfiniteStamina = false;
    [[configurable]] bool m_NoFallDamage = false;
    [[configurable]] float m_SpeedMultiplier = 1.0f;
    [[configurable]] float m_JumpMultiplier = 1.0f;
    [[configurable]] float m_DamageMultiplier = 1.0f;

    // ========================================
    // World Tab Configuration
    // ========================================
    [[configurable]] bool m_WorldEnabled = false;
    [[configurable]] float m_TimeScale = 1.0f;
    [[configurable]] float m_Gravity = -980.0f;
    [[configurable]] bool m_FreezeTime = false;

    // ========================================
    // Frame Event Handler
    // ========================================
    void OnFrameEvent(const Broadsword::Frame& frame);

    // ========================================
    // Hook Handlers
    // ========================================
    void RegisterHooks(Broadsword::ModContext& ctx);
    std::vector<size_t> m_HookIds;
};

} // namespace Enhancer
