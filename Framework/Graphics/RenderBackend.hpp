#pragma once

#include <d3d11.h>
#include <d3d12.h>
#include <dxgi.h>
#include <memory>

namespace Broadsword
{

// Abstract render backend interface
// Supports both DX11 and DX12
class RenderBackend
{
public:
    enum class API
    {
        Unknown,
        DX11,
        DX12
    };

    virtual ~RenderBackend() = default;

    // Get the API type
    virtual API GetAPI() const = 0;

    // Initialize ImGui for this backend
    virtual bool InitializeImGui(IDXGISwapChain* swapChain) = 0;

    // Shutdown ImGui
    virtual void ShutdownImGui() = 0;

    // New frame
    virtual void NewFrame() = 0;

    // Render ImGui draw data
    virtual void RenderDrawData() = 0;

    // Handle resize
    virtual void OnResize(IDXGISwapChain* swapChain, UINT width, UINT height) = 0;

    // Check if initialized
    virtual bool IsInitialized() const = 0;
};

// Factory to create the appropriate backend
std::unique_ptr<RenderBackend> CreateRenderBackend(RenderBackend::API api);

} // namespace Broadsword
