#pragma once

#include "RenderBackend.hpp"

namespace Broadsword
{

class DX11Backend : public RenderBackend
{
public:
    DX11Backend() = default;
    ~DX11Backend() override;

    API GetAPI() const override { return API::DX11; }

    bool InitializeImGui(IDXGISwapChain* swapChain) override;
    void ShutdownImGui() override;
    void NewFrame() override;
    void RenderDrawData() override;
    void OnResize(IDXGISwapChain* swapChain, UINT width, UINT height) override;
    bool IsInitialized() const override { return m_Initialized; }

private:
    bool m_Initialized = false;
    ID3D11Device* m_Device = nullptr;
    ID3D11DeviceContext* m_DeviceContext = nullptr;
    ID3D11RenderTargetView* m_RenderTargetView = nullptr;
};

} // namespace Broadsword
