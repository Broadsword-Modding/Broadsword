#pragma once

#include <MinHook.h>
#include <functional>

namespace Broadsword::Foundation
{

// InlineHook - Function hooking using MinHook
//
// Use this for hooking non-virtual functions (e.g., ProcessEvent)
// For virtual functions (vtable hooks), use VTableHook instead
class InlineHook
{
public:
    // Initialize MinHook library
    static bool Initialize();

    // Shutdown MinHook library
    static void Shutdown();

    // Create a hook
    template <typename T>
    static bool Create(void* target, T detour, T* original)
    {
        if (MH_CreateHook(target, (void*)detour, (void**)original) != MH_OK)
        {
            return false;
        }

        return MH_EnableHook(target) == MH_OK;
    }

    // Remove a hook
    static bool Remove(void* target);

    // Enable a hook
    static bool Enable(void* target);

    // Disable a hook
    static bool Disable(void* target);

private:
    static bool s_Initialized;
};

} // namespace Broadsword::Foundation
