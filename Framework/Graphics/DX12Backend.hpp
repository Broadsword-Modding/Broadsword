#pragma once

#include "RenderBackend.hpp"
#include <vector>

namespace Broadsword
{

class DX12Backend : public RenderBackend
{
public:
    DX12Backend() = default;
    ~DX12Backend() override;

    API GetAPI() const override { return API::DX12; }

    bool InitializeImGui(IDXGISwapChain* swapChain) override;
    void ShutdownImGui() override;
    void NewFrame() override;
    void RenderDrawData() override;
    void OnResize(IDXGISwapChain* swapChain, UINT width, UINT height) override;
    bool IsInitialized() const override { return m_Initialized; }

private:
    struct FrameContext
    {
        ID3D12CommandAllocator* CommandAllocator = nullptr;
        ID3D12Resource* BackBuffer = nullptr;
        D3D12_CPU_DESCRIPTOR_HANDLE RtvHandle{};
    };

    bool CreateRenderTargets(IDXGISwapChain* swapChain);
    void CleanupRenderTargets();

    bool m_Initialized = false;
    ID3D12Device* m_Device = nullptr;
    ID3D12DescriptorHeap* m_RtvDescHeap = nullptr;
    ID3D12DescriptorHeap* m_SrvDescHeap = nullptr;
    ID3D12CommandQueue* m_CommandQueue = nullptr;
    ID3D12GraphicsCommandList* m_CommandList = nullptr;

    std::vector<FrameContext> m_FrameContexts;
    UINT m_BufferCount = 0;
    UINT m_RtvDescriptorSize = 0;
};

} // namespace Broadsword
