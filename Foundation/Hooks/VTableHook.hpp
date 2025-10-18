#pragma once

#include "kiero/kiero.h"
#include <cstdint>
#include <functional>

namespace Broadsword::Foundation
{

// VTableHook - Virtual Method Table hooking using kiero
//
// Kiero supports multiple rendering APIs with auto-detection:
// - D3D11, D3D12, D3D9, D3D10, OpenGL, Vulkan
//
// For Half Sword (UE5), we use Auto mode to detect DX11 or DX12
class VTableHook
{
public:
    enum class RenderAPI
    {
        Auto,   // Auto-detect (recommended)
        DX9,
        DX10,
        DX11,
        DX12,
        OpenGL,
        Vulkan
    };

    // Initialize kiero with specified render API
    static bool Initialize(RenderAPI api = RenderAPI::Auto);

    // Shutdown kiero (unbinds all hooks)
    static void Shutdown();

    // Check if kiero is initialized
    static bool IsInitialized();

    // Get detected render API
    static RenderAPI GetDetectedAPI();

    // Bind a hook to a vtable index
    // Returns true if successful
    template <typename T>
    static bool Bind(uint16_t index, T** original, T detour)
    {
        kiero::Status::Enum status = kiero::bind(index, (void**)original, (void*)detour);
        return status == kiero::Status::Success;
    }

    // Unbind a hook from a vtable index
    static void Unbind(uint16_t index);

private:
    static RenderAPI s_DetectedAPI;
    static bool s_Initialized;

    static kiero::RenderType::Enum ToKieroType(RenderAPI api);
    static RenderAPI FromKieroType(kiero::RenderType::Enum type);
};

} // namespace Broadsword::Foundation
