#include "InlineHook.hpp"

namespace Broadsword::Foundation
{

bool InlineHook::s_Initialized = false;

bool InlineHook::Initialize()
{
    if (s_Initialized)
    {
        return true;
    }

    if (MH_Initialize() == MH_OK)
    {
        s_Initialized = true;
        return true;
    }

    return false;
}

void InlineHook::Shutdown()
{
    if (s_Initialized)
    {
        MH_Uninitialize();
        s_Initialized = false;
    }
}

bool InlineHook::Remove(void* target)
{
    return MH_RemoveHook(target) == MH_OK;
}

bool InlineHook::Enable(void* target)
{
    return MH_EnableHook(target) == MH_OK;
}

bool InlineHook::Disable(void* target)
{
    return MH_DisableHook(target) == MH_OK;
}

} // namespace Broadsword::Foundation
