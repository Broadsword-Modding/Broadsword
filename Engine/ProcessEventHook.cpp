#include "ProcessEventHook.hpp"
#include "../Foundation/Hooks/VTableHook.hpp"
#include <iostream>

namespace Broadsword {

ProcessEventHook& ProcessEventHook::Get() {
    static ProcessEventHook instance;
    return instance;
}

void ProcessEventHook::Initialize() {
    if (m_Initialized) {
        std::cerr << "[ProcessEventHook] Already initialized\n";
        return;
    }

    std::cout << "[ProcessEventHook] Initializing ProcessEvent hook...\n";

    // Get any UObject instance to access ProcessEvent VMT
    // We'll hook the ProcessEvent function in the UObject VMT
    // For now, we'll use a placeholder approach - this would need
    // to be implemented with actual VMT hooking once we have
    // access to a UObject instance

    // TODO: Implement actual VMT hook
    // For now, just mark as initialized
    m_Initialized = true;

    std::cout << "[ProcessEventHook] ProcessEvent hook initialized\n";
}

void ProcessEventHook::Shutdown() {
    if (!m_Initialized) {
        return;
    }

    std::cout << "[ProcessEventHook] Shutting down ProcessEvent hook...\n";

    // Restore original ProcessEvent
    if (m_OriginalProcessEvent) {
        // TODO: Restore VMT
        m_OriginalProcessEvent = nullptr;
    }

    // Clear all hooks
    ClearAllHooks();

    m_Initialized = false;

    std::cout << "[ProcessEventHook] ProcessEvent hook shutdown complete\n";
}

size_t ProcessEventHook::AddHook(std::string_view functionName, HookCallback callback) {
    size_t hookId = m_NextHookId++;

    Hook hook;
    hook.id = hookId;
    hook.functionName = functionName;
    hook.callback = std::move(callback);

    std::string funcNameStr(functionName);
    m_Hooks[funcNameStr].push_back(std::move(hook));

    std::cout << "[ProcessEventHook] Added hook " << hookId << " for function: " << functionName << "\n";

    return hookId;
}

void ProcessEventHook::RemoveHook(size_t hookId) {
    for (auto& [functionName, hooks] : m_Hooks) {
        auto it = std::remove_if(hooks.begin(), hooks.end(),
            [hookId](const Hook& hook) { return hook.id == hookId; });

        if (it != hooks.end()) {
            hooks.erase(it, hooks.end());
            std::cout << "[ProcessEventHook] Removed hook " << hookId << "\n";
            return;
        }
    }

    std::cerr << "[ProcessEventHook] Hook " << hookId << " not found\n";
}

void ProcessEventHook::RemoveHooksForFunction(std::string_view functionName) {
    std::string funcNameStr(functionName);
    auto it = m_Hooks.find(funcNameStr);

    if (it != m_Hooks.end()) {
        size_t count = it->second.size();
        m_Hooks.erase(it);
        std::cout << "[ProcessEventHook] Removed " << count << " hooks for function: " << functionName << "\n";
    }
}

void ProcessEventHook::ClearAllHooks() {
    size_t totalHooks = 0;
    for (const auto& [functionName, hooks] : m_Hooks) {
        totalHooks += hooks.size();
    }

    m_Hooks.clear();

    std::cout << "[ProcessEventHook] Cleared all " << totalHooks << " hooks\n";
}

size_t ProcessEventHook::GetHookCount(std::string_view functionName) const {
    std::string funcNameStr(functionName);
    auto it = m_Hooks.find(funcNameStr);

    if (it != m_Hooks.end()) {
        return it->second.size();
    }

    return 0;
}

void ProcessEventHook::ProcessEventDetour(SDK::UObject* object, SDK::UFunction* function, void* params) {
    if (!function) {
        // No function, call original
        if (m_OriginalProcessEvent) {
            m_OriginalProcessEvent(object, function, params);
        }
        return;
    }

    // Get function name
    std::string functionName = function->GetName();

    // Check if we have hooks for this function
    auto it = m_Hooks.find(functionName);
    bool shouldCallOriginal = true;

    if (it != m_Hooks.end()) {
        // Call all hooks for this function
        for (const auto& hook : it->second) {
            try {
                bool result = hook.callback(object, function, params);
                if (!result) {
                    // Hook returned false, don't call original
                    shouldCallOriginal = false;
                }
            } catch (const std::exception& e) {
                std::cerr << "[ProcessEventHook] Exception in hook for " << functionName << ": " << e.what() << "\n";
            } catch (...) {
                std::cerr << "[ProcessEventHook] Unknown exception in hook for " << functionName << "\n";
            }
        }
    }

    // Call original function if not blocked
    if (shouldCallOriginal && m_OriginalProcessEvent) {
        m_OriginalProcessEvent(object, function, params);
    }
}

void ProcessEventHook::ProcessEventTrampoline(SDK::UObject* object, SDK::UFunction* function, void* params) {
    ProcessEventHook::Get().ProcessEventDetour(object, function, params);
}

} // namespace Broadsword
