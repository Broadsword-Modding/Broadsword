#include "VTableHook.hpp"

namespace Broadsword::Foundation
{

bool VTableHook::s_Initialized = false;
VTableHook::RenderAPI VTableHook::s_DetectedAPI = RenderAPI::Auto;

bool VTableHook::Initialize(RenderAPI api)
{
    if (s_Initialized)
    {
        return true;
    }

    kiero::RenderType::Enum kieroType = ToKieroType(api);
    kiero::Status::Enum status = kiero::init(kieroType);

    if (status == kiero::Status::Success)
    {
        s_Initialized = true;

        // Store detected API
        // Note: kiero doesn't expose what it detected in Auto mode,
        // but we can infer from hook success
        s_DetectedAPI = api;

        return true;
    }

    return false;
}

void VTableHook::Shutdown()
{
    if (s_Initialized)
    {
        kiero::shutdown();
        s_Initialized = false;
    }
}

bool VTableHook::IsInitialized()
{
    return s_Initialized;
}

VTableHook::RenderAPI VTableHook::GetDetectedAPI()
{
    return s_DetectedAPI;
}

void VTableHook::Unbind(uint16_t index)
{
    kiero::unbind(index);
}

kiero::RenderType::Enum VTableHook::ToKieroType(RenderAPI api)
{
    switch (api)
    {
    case RenderAPI::Auto:
        return kiero::RenderType::Auto;
    case RenderAPI::DX9:
        return kiero::RenderType::D3D9;
    case RenderAPI::DX10:
        return kiero::RenderType::D3D10;
    case RenderAPI::DX11:
        return kiero::RenderType::D3D11;
    case RenderAPI::DX12:
        return kiero::RenderType::D3D12;
    case RenderAPI::OpenGL:
        return kiero::RenderType::OpenGL;
    case RenderAPI::Vulkan:
        return kiero::RenderType::Vulkan;
    default:
        return kiero::RenderType::Auto;
    }
}

VTableHook::RenderAPI VTableHook::FromKieroType(kiero::RenderType::Enum type)
{
    switch (type)
    {
    case kiero::RenderType::D3D9:
        return RenderAPI::DX9;
    case kiero::RenderType::D3D10:
        return RenderAPI::DX10;
    case kiero::RenderType::D3D11:
        return RenderAPI::DX11;
    case kiero::RenderType::D3D12:
        return RenderAPI::DX12;
    case kiero::RenderType::OpenGL:
        return RenderAPI::OpenGL;
    case kiero::RenderType::Vulkan:
        return RenderAPI::Vulkan;
    default:
        return RenderAPI::Auto;
    }
}

} // namespace Broadsword::Foundation
