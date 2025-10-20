#pragma once

#include "../Engine/SDK/SDK.hpp"
#include <functional>
#include <unordered_map>
#include <vector>
#include <string>
#include <string_view>

namespace Broadsword {

/**
 * ProcessEventHook - Hook UObject::ProcessEvent for function interception
 *
 * Allows mods to intercept any UFunction call in the game.
 * Uses VMT (Virtual Method Table) hooking to redirect ProcessEvent calls.
 *
 * Features:
 * - Hook by function name (e.g., "TakeDamage", "OnDeath")
 * - Multiple hooks per function
 * - Return false from callback to block original function
 * - Type-safe parameter access via void* casting
 *
 * Thread Safety:
 * - All methods MUST be called from game thread
 * - ProcessEvent is called on game thread
 * - No synchronization needed (single-threaded)
 *
 * Usage:
 *   // Hook TakeDamage function
 *   size_t hookId = ProcessEventHook::Get().AddHook(
 *       "TakeDamage",
 *       [](SDK::UObject* object, SDK::UFunction* function, void* params) {
 *           // Cast params to function-specific struct
 *           // auto* damageParams = static_cast<SDK::FTakeDamageParams*>(params);
 *
 *           // Modify or inspect params here
 *
 *           return true; // true = call original, false = block
 *       }
 *   );
 *
 *   // Later: remove hook
 *   ProcessEventHook::Get().RemoveHook(hookId);
 */
class ProcessEventHook {
public:
    using HookCallback = std::function<bool(SDK::UObject*, SDK::UFunction*, void*)>;

    static ProcessEventHook& Get();

    ProcessEventHook(const ProcessEventHook&) = delete;
    ProcessEventHook& operator=(const ProcessEventHook&) = delete;

    /**
     * Initialize ProcessEvent hook
     *
     * Must be called once during framework startup.
     * Hooks UObject::ProcessEvent via VMT.
     */
    void Initialize();

    /**
     * Shutdown ProcessEvent hook
     *
     * Restores original ProcessEvent function.
     * Must be called during framework shutdown.
     */
    void Shutdown();

    /**
     * Add hook for a function name
     *
     * @param functionName Name of UFunction to hook (e.g., "TakeDamage")
     * @param callback Hook callback function
     * @return Hook ID for later removal
     */
    size_t AddHook(std::string_view functionName, HookCallback callback);

    /**
     * Remove hook by ID
     *
     * @param hookId Hook ID returned from AddHook
     */
    void RemoveHook(size_t hookId);

    /**
     * Remove all hooks for a function name
     *
     * @param functionName Name of UFunction
     */
    void RemoveHooksForFunction(std::string_view functionName);

    /**
     * Remove all hooks
     */
    void ClearAllHooks();

    /**
     * Get number of hooks for a function
     *
     * @param functionName Name of UFunction
     * @return Number of registered hooks
     */
    size_t GetHookCount(std::string_view functionName) const;

private:
    ProcessEventHook() = default;

    /**
     * Hook function that replaces ProcessEvent
     *
     * This is the actual hooked function that gets called instead of
     * the original ProcessEvent. It invokes all registered callbacks
     * and conditionally calls the original function.
     *
     * @param object UObject instance
     * @param function UFunction being called
     * @param params Function parameters
     */
    void ProcessEventDetour(SDK::UObject* object, SDK::UFunction* function, void* params);

    /**
     * Original ProcessEvent function pointer
     * Set during Initialize() via VMT hook
     */
    using ProcessEventFunc = void (*)(SDK::UObject*, SDK::UFunction*, void*);
    ProcessEventFunc m_OriginalProcessEvent = nullptr;

    /**
     * Hook storage
     */
    struct Hook {
        size_t id;
        std::string functionName;
        HookCallback callback;
    };

    std::unordered_map<std::string, std::vector<Hook>> m_Hooks;
    size_t m_NextHookId = 1;

    bool m_Initialized = false;

    /**
     * Static trampoline for ProcessEvent hook
     * Needed because we can't use member function pointers for VMT hooks
     */
    static void ProcessEventTrampoline(SDK::UObject* object, SDK::UFunction* function, void* params);
};

} // namespace Broadsword
