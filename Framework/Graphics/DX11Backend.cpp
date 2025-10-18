#include "DX11Backend.hpp"
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

namespace Broadsword
{

DX11Backend::~DX11Backend()
{
    ShutdownImGui();
}

bool DX11Backend::InitializeImGui(IDXGISwapChain* swapChain)
{
    if (m_Initialized)
    {
        return true;
    }

    if (!swapChain)
    {
        return false;
    }

    // Get device and context from swap chain
    if (FAILED(swapChain->GetDevice(__uuidof(ID3D11Device), (void**)&m_Device)))
    {
        return false;
    }

    m_Device->GetImmediateContext(&m_DeviceContext);

    // Create render target view
    ID3D11Texture2D* backBuffer = nullptr;
    if (FAILED(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer)))
    {
        return false;
    }

    if (FAILED(m_Device->CreateRenderTargetView(backBuffer, nullptr, &m_RenderTargetView)))
    {
        backBuffer->Release();
        return false;
    }

    backBuffer->Release();

    // Initialize ImGui DX11 backend
    if (!ImGui_ImplDX11_Init(m_Device, m_DeviceContext))
    {
        return false;
    }

    m_Initialized = true;
    return true;
}

void DX11Backend::ShutdownImGui()
{
    if (!m_Initialized)
    {
        return;
    }

    ImGui_ImplDX11_Shutdown();

    if (m_RenderTargetView)
    {
        m_RenderTargetView->Release();
        m_RenderTargetView = nullptr;
    }

    if (m_DeviceContext)
    {
        m_DeviceContext->Release();
        m_DeviceContext = nullptr;
    }

    if (m_Device)
    {
        m_Device->Release();
        m_Device = nullptr;
    }

    m_Initialized = false;
}

void DX11Backend::NewFrame()
{
    if (m_Initialized)
    {
        ImGui_ImplDX11_NewFrame();
    }
}

void DX11Backend::RenderDrawData()
{
    if (m_Initialized && m_DeviceContext)
    {
        // Set render target
        m_DeviceContext->OMSetRenderTargets(1, &m_RenderTargetView, nullptr);

        // Render ImGui
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }
}

void DX11Backend::OnResize(IDXGISwapChain* swapChain, UINT width, UINT height)
{
    if (!m_Initialized || !swapChain)
    {
        return;
    }

    // Release old render target view
    if (m_RenderTargetView)
    {
        m_RenderTargetView->Release();
        m_RenderTargetView = nullptr;
    }

    // Create new render target view
    ID3D11Texture2D* backBuffer = nullptr;
    if (SUCCEEDED(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer)))
    {
        m_Device->CreateRenderTargetView(backBuffer, nullptr, &m_RenderTargetView);
        backBuffer->Release();
    }
}

} // namespace Broadsword
