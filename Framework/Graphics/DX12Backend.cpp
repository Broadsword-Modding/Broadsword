#include "DX12Backend.hpp"
#include "../../Foundation/Hooks/d3dx12.h"
#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>

namespace Broadsword
{

DX12Backend::~DX12Backend()
{
    ShutdownImGui();
}

bool DX12Backend::InitializeImGui(IDXGISwapChain* swapChain)
{
    if (m_Initialized)
    {
        return true;
    }

    if (!swapChain)
    {
        return false;
    }

    // Get device from swap chain
    if (FAILED(swapChain->GetDevice(__uuidof(ID3D12Device), (void**)&m_Device)))
    {
        return false;
    }

    // Get buffer count
    DXGI_SWAP_CHAIN_DESC desc;
    swapChain->GetDesc(&desc);
    m_BufferCount = desc.BufferCount;
    m_FrameContexts.resize(m_BufferCount);

    // Create descriptor heap for RTV
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.NumDescriptors = m_BufferCount;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 1;

    if (FAILED(m_Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_RtvDescHeap))))
    {
        return false;
    }

    m_RtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // Create descriptor heap for SRV (ImGui textures)
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.NumDescriptors = 1;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    if (FAILED(m_Device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_SrvDescHeap))))
    {
        return false;
    }

    // Create command allocators and get back buffers
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RtvDescHeap->GetCPUDescriptorHandleForHeapStart());

    for (UINT i = 0; i < m_BufferCount; i++)
    {
        if (FAILED(m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                     IID_PPV_ARGS(&m_FrameContexts[i].CommandAllocator))))
        {
            return false;
        }

        if (FAILED(swapChain->GetBuffer(i, IID_PPV_ARGS(&m_FrameContexts[i].BackBuffer))))
        {
            return false;
        }

        m_Device->CreateRenderTargetView(m_FrameContexts[i].BackBuffer, nullptr, rtvHandle);
        m_FrameContexts[i].RtvHandle = rtvHandle;
        rtvHandle.Offset(1, m_RtvDescriptorSize);
    }

    // Create command list
    if (FAILED(m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_FrameContexts[0].CommandAllocator,
                                            nullptr, IID_PPV_ARGS(&m_CommandList))))
    {
        return false;
    }

    m_CommandList->Close();

    // Get command queue (try to find from swap chain or create new one)
    // Note: In practice, we should get this from the swap chain's device
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

    if (FAILED(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CommandQueue))))
    {
        return false;
    }

    // Initialize ImGui DX12 backend
    if (!ImGui_ImplDX12_Init(m_Device, m_BufferCount, DXGI_FORMAT_R8G8B8A8_UNORM, m_SrvDescHeap,
                              m_SrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
                              m_SrvDescHeap->GetGPUDescriptorHandleForHeapStart()))
    {
        return false;
    }

    m_Initialized = true;
    return true;
}

void DX12Backend::ShutdownImGui()
{
    if (!m_Initialized)
    {
        return;
    }

    ImGui_ImplDX12_Shutdown();

    CleanupRenderTargets();

    if (m_CommandList)
    {
        m_CommandList->Release();
        m_CommandList = nullptr;
    }

    if (m_CommandQueue)
    {
        m_CommandQueue->Release();
        m_CommandQueue = nullptr;
    }

    if (m_SrvDescHeap)
    {
        m_SrvDescHeap->Release();
        m_SrvDescHeap = nullptr;
    }

    if (m_RtvDescHeap)
    {
        m_RtvDescHeap->Release();
        m_RtvDescHeap = nullptr;
    }

    if (m_Device)
    {
        m_Device->Release();
        m_Device = nullptr;
    }

    m_Initialized = false;
}

void DX12Backend::NewFrame()
{
    if (m_Initialized)
    {
        ImGui_ImplDX12_NewFrame();
    }
}

void DX12Backend::RenderDrawData()
{
    if (!m_Initialized || !m_CommandList)
    {
        return;
    }

    // Get current back buffer index
    IDXGISwapChain3* swapChain3 = nullptr;
    // Note: We'd need to store the swap chain reference to get current back buffer index
    // For now, use frame context 0
    UINT backBufferIdx = 0; // Should be: swapChain3->GetCurrentBackBufferIndex();

    FrameContext& frameCtx = m_FrameContexts[backBufferIdx];

    // Reset command allocator and list
    frameCtx.CommandAllocator->Reset();
    m_CommandList->Reset(frameCtx.CommandAllocator, nullptr);

    // Transition back buffer to render target state
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = frameCtx.BackBuffer;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

    m_CommandList->ResourceBarrier(1, &barrier);

    // Set render target
    m_CommandList->OMSetRenderTargets(1, &frameCtx.RtvHandle, FALSE, nullptr);
    m_CommandList->SetDescriptorHeaps(1, &m_SrvDescHeap);

    // Render ImGui
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_CommandList);

    // Transition back to present state
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    m_CommandList->ResourceBarrier(1, &barrier);

    // Execute command list
    m_CommandList->Close();
    m_CommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&m_CommandList);
}

void DX12Backend::OnResize(IDXGISwapChain* swapChain, UINT width, UINT height)
{
    if (!m_Initialized || !swapChain)
    {
        return;
    }

    CleanupRenderTargets();
    CreateRenderTargets(swapChain);
}

bool DX12Backend::CreateRenderTargets(IDXGISwapChain* swapChain)
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RtvDescHeap->GetCPUDescriptorHandleForHeapStart());

    for (UINT i = 0; i < m_BufferCount; i++)
    {
        if (FAILED(swapChain->GetBuffer(i, IID_PPV_ARGS(&m_FrameContexts[i].BackBuffer))))
        {
            return false;
        }

        m_Device->CreateRenderTargetView(m_FrameContexts[i].BackBuffer, nullptr, rtvHandle);
        m_FrameContexts[i].RtvHandle = rtvHandle;
        rtvHandle.Offset(1, m_RtvDescriptorSize);
    }

    return true;
}

void DX12Backend::CleanupRenderTargets()
{
    for (auto& frameCtx : m_FrameContexts)
    {
        if (frameCtx.BackBuffer)
        {
            frameCtx.BackBuffer->Release();
            frameCtx.BackBuffer = nullptr;
        }

        if (frameCtx.CommandAllocator)
        {
            frameCtx.CommandAllocator->Release();
            frameCtx.CommandAllocator = nullptr;
        }
    }
}

} // namespace Broadsword
