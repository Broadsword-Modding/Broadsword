#pragma once

#include "../Engine/ProcessEventHook.hpp"
#include <functional>
#include <string_view>
#include <vector>

namespace Broadsword {

/**
 * HookContext - Mod API for ProcessEvent hooking
 *
 * Provides type-safe interface for mods to hook UFunction calls.
 * Automatically manages hook lifecycle and cleanup.
 *
 * Features:
 * - Simple Hook() method with function name and callback
 * - Type-safe parameter casting via template overload
 * - Automatic unhook on mod unload
 * - Multiple hooks per function supported
 *
 * Usage:
 *   // In mod OnRegister:
 *   void OnRegister(ModContext& ctx) override {
 *       // Basic hook with void* params
 *       m_HookId = ctx.hooks.Hook("TakeDamage",
 *           [this](SDK::UObject* obj, void* params) {
 *               // Cast params manually
 *               return true; // true = call original
 *           });
 *
 *       // Type-safe hook with template
 *       m_HookId = ctx.hooks.Hook<SDK::FTakeDamageParams>("TakeDamage",
 *           [this](SDK::UObject* obj, SDK::FTakeDamageParams* params) {
 *               // params is already typed correctly
 *               params->DamageAmount *= 0.5f; // Reduce damage by 50%
 *               return true;
 *           });
 *   }
 *
 *   // In mod OnUnregister:
 *   void OnUnregister() override {
 *       // Hooks are automatically removed, but you can manually unhook:
 *       ctx.hooks.Unhook(m_HookId);
 *   }
 */
class HookContext {
public:
    HookContext() = default;
    ~HookContext();

    /**
     * Hook a UFunction by name
     *
     * @param functionName Name of UFunction to hook (e.g., "TakeDamage")
     * @param callback Hook callback (return false to block original)
     * @return Hook ID for later removal
     */
    size_t Hook(
        std::string_view functionName,
        std::function<bool(SDK::UObject*, void*)> callback
    );

    /**
     * Hook a UFunction by name with type-safe parameters
     *
     * @tparam ParamsType Function parameter struct type
     * @param functionName Name of UFunction to hook
     * @param callback Type-safe hook callback
     * @return Hook ID for later removal
     */
    template<typename ParamsType>
    size_t Hook(
        std::string_view functionName,
        std::function<bool(SDK::UObject*, ParamsType*)> callback
    ) {
        // Wrap type-safe callback in generic callback
        auto wrappedCallback = [callback](SDK::UObject* obj, SDK::UFunction* func, void* params) {
            auto* typedParams = static_cast<ParamsType*>(params);
            return callback(obj, typedParams);
        };

        size_t hookId = ProcessEventHook::Get().AddHook(functionName, wrappedCallback);
        m_RegisteredHooks.push_back(hookId);
        return hookId;
    }

    /**
     * Remove a hook by ID
     *
     * @param hookId Hook ID returned from Hook()
     */
    void Unhook(size_t hookId);

    /**
     * Remove all hooks registered by this context
     *
     * Called automatically during destruction.
     */
    void UnhookAll();

private:
    std::vector<size_t> m_RegisteredHooks;
};

} // namespace Broadsword
