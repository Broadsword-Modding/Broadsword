#include "EnhancerMod.hpp"

namespace Enhancer {

EnhancerMod::EnhancerMod() = default;

void EnhancerMod::OnRegister(Broadsword::ModContext& ctx) {
    // Subscribe to frame events for game logic updates
    // ctx.events.Subscribe<OnFrameEvent>([this](const auto& evt) {
    //     OnFrameEvent(evt.frame);
    // });

    // Register ProcessEvent hooks
    RegisterHooks(ctx);

    // TODO: Log registration (Logger requires source location macros)
    // ctx.log.Info("Enhancer mod registered");
}

void EnhancerMod::OnUnregister() {
    // Cleanup handled automatically by framework
}

void EnhancerMod::OnFrameEvent(const Broadsword::Frame& frame) {
    // TODO: Implement frame-based updates
    // - Apply player modifications
    // - Apply world modifications
    // - Handle spawning logic
}

void EnhancerMod::RegisterHooks(Broadsword::ModContext& ctx) {
    // TODO: Register ProcessEvent hooks
    // Example:
    // auto hookId = ctx.hooks.Hook("FunctionName", [](SDK::UObject* obj, void* params) {
    //     // Hook logic
    //     return false; // false = don't block original
    // });
    // m_HookIds.push_back(hookId);
}

} // namespace Enhancer

// ========================================
// Mod Export Functions
// ========================================
extern "C" __declspec(dllexport) Broadsword::Mod* CreateMod() {
    return new Enhancer::EnhancerMod();
}

extern "C" __declspec(dllexport) void DestroyMod(Broadsword::Mod* mod) {
    delete mod;
}
