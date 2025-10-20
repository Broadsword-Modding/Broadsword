#include "HookContext.hpp"
#include <algorithm>

namespace Broadsword {

HookContext::~HookContext() {
    UnhookAll();
}

size_t HookContext::Hook(
    std::string_view functionName,
    std::function<bool(SDK::UObject*, void*)> callback
) {
    // Wrap callback to match ProcessEventHook signature
    auto wrappedCallback = [callback](SDK::UObject* obj, SDK::UFunction* func, void* params) {
        return callback(obj, params);
    };

    size_t hookId = ProcessEventHook::Get().AddHook(functionName, wrappedCallback);
    m_RegisteredHooks.push_back(hookId);
    return hookId;
}

void HookContext::Unhook(size_t hookId) {
    ProcessEventHook::Get().RemoveHook(hookId);

    // Remove from our list
    auto it = std::remove(m_RegisteredHooks.begin(), m_RegisteredHooks.end(), hookId);
    m_RegisteredHooks.erase(it, m_RegisteredHooks.end());
}

void HookContext::UnhookAll() {
    for (size_t hookId : m_RegisteredHooks) {
        ProcessEventHook::Get().RemoveHook(hookId);
    }

    m_RegisteredHooks.clear();
}

} // namespace Broadsword
