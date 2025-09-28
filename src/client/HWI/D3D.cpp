//
// Created by fionaw on 22/09/2025.
//

#include "HWI/D3D.h"
#include "Headers/Helper.h"
#include "System/Win32App.h"
#include "System/DebugOutputRedirector.h"

// Helper function for acquiring the first available hardware adapter that supports Direct3D 12.
// If no such adapter can be found, *ppAdapter will be set to nullptr.
_Use_decl_annotations_
void getHardwareAdapter(
    IDXGIFactory1* pFactory,
    IDXGIAdapter1** ppAdapter,
    bool requestHighPerformanceAdapter = false)
{
    *ppAdapter = nullptr;

    ComPtr<IDXGIAdapter1> adapter;

    ComPtr<IDXGIFactory6> factory6;
    if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
    {
        for (
            UINT adapterIndex = 0;
            SUCCEEDED(factory6->EnumAdapterByGpuPreference(
                adapterIndex,
                requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
                IID_PPV_ARGS(&adapter)));
            ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Don't select the Basic Render Driver adapter.
                // If you want a software adapter, pass in "/warp" on the command line.
                continue;
            }

            // Check to see whether the adapter supports Direct3D 12, but don't create the
            // actual device yet.
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
            {
                break;
            }
        }
    }

    if(adapter.Get() == nullptr)
    {
        for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Don't select the Basic Render Driver adapter.
                // If you want a software adapter, pass in "/warp" on the command line.
                continue;
            }

            // Check to see whether the adapter supports Direct3D 12, but don't create the
            // actual device yet.
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
            {
                break;
            }
        }
    }

    *ppAdapter = adapter.Detach();
}

D3D::~D3D()
{
    if (m_logFile.is_open())
        m_logFile.close();
}

void D3D::Init(size_t width, size_t height)
{
    UINT dxgiFactoryFlags = 0;

#ifndef NDEBUG
    // Enable the debug layer (requires the Graphics Tools "optional feature").
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();

            // Enable additional debug layers.
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }
#endif

    ComPtr<IDXGIFactory4> factory;
    V(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

    if (m_useWarpDevice)
    {
        ComPtr<IDXGIAdapter> warpAdapter;
        V(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

        V(D3D12CreateDevice(
            warpAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_device)
            ));
    }
    else
    {
        ComPtr<IDXGIAdapter1> hardwareAdapter;
        getHardwareAdapter(factory.Get(), &hardwareAdapter);

        V(D3D12CreateDevice(
            hardwareAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_device)
            ));
    }

#ifdef _DEBUG
    if (SUCCEEDED(m_device.As(&m_infoQueue)))
    {
        m_logFile.open("d3d12log.txt");

        V(m_infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE));
        V(m_infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE));
        V(m_infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, FALSE));

        DWORD cookie = 0;
        HRESULT hr = m_infoQueue->RegisterMessageCallback(
            DebugMessageCallback,
            D3D12_MESSAGE_CALLBACK_FLAG_NONE,
            &std::cout,   // passed to callback as `context`
            &cookie);
        if (FAILED(hr))
        {
            std::cerr << "Failed to register debug callback to window. HRESULT = " << std::hex << hr << std::endl;
        }

        hr = m_infoQueue->RegisterMessageCallback(
            DebugMessageCallback,
            D3D12_MESSAGE_CALLBACK_FLAG_NONE,
            &m_logFile,   // passed to callback as `context`
            &cookie);
        if (FAILED(hr))
        {
            std::cerr << "Failed to register debug callback to log file. HRESULT = " << std::hex << hr << std::endl;
        }

        // You can also filter messages if it's too noisy
    }
#endif

    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    V(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = c_FrameCount;
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swapChain;
    V(factory->CreateSwapChainForHwnd(
        m_commandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
        Win32App::GetHwnd(),
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain
        ));

    // This sample does not support fullscreen transitions.
    V(factory->MakeWindowAssociation(Win32App::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));

    V(swapChain.As(&m_swapChain));
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // Create descriptor heaps.
    {
        // Describe and create a render target view (RTV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = c_FrameCount;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        V(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

        m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }

    // Create frame resources.
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        // Create a RTV for each frame.
        for (UINT n = 0; n < c_FrameCount; n++)
        {
            V(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
            m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_rtvDescriptorSize);
        }
    }

    // Create synchronization objects and wait until assets have been uploaded to the GPU.
    {
        V(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
        m_fenceValue = 1;

        // Create an event handle to use for frame synchronization.
        m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_fenceEvent == nullptr)
        {
            V(HRESULT_FROM_WIN32(GetLastError()));
        }
    }

    for (int i = 0; i < c_FrameCount; i++)
        m_frameBufferFences[i] = 0;
}

ComPtr<ID3D12GraphicsCommandList> D3D::CreateCmdList(ID3D12CommandAllocator* allocator) const
{
    ComPtr<ID3D12GraphicsCommandList> cmdList = nullptr;
    V(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator, nullptr, IID_PPV_ARGS(&cmdList)));

    return cmdList;
}

ComPtr<ID3D12CommandAllocator> D3D::CreateAllocator(const D3D12_COMMAND_LIST_TYPE type) const
{
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    V(m_device->CreateCommandAllocator(type, IID_PPV_ARGS(&commandAllocator)));
    return commandAllocator;
}

ComPtr<ID3D12GraphicsCommandList> D3D::GetAvailableCmdList(const D3D12_COMMAND_LIST_TYPE type)
{
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    ComPtr<ID3D12GraphicsCommandList> cmdList;

    if (!m_commandAllocatorQueue.empty() && IsFenceComplete(m_commandAllocatorQueue.front().Fence))
    {
        commandAllocator = m_commandAllocatorQueue.front().Allocator;
        m_commandAllocatorQueue.pop();

        V(commandAllocator->Reset());
    }
    else
    {
        commandAllocator = CreateAllocator(type);
    }

    if (!m_commandListQueue.empty())
    {
        cmdList = m_commandListQueue.front();
        m_commandListQueue.pop();

        V(cmdList->Reset(commandAllocator.Get(), nullptr));
    }
    else
    {
        cmdList = CreateCmdList(commandAllocator.Get());
    }

    // Associate the command allocator with the command list so that it can be
    // retrieved when the command list is executed.
    V(cmdList->SetPrivateDataInterface(__uuidof(ID3D12CommandAllocator), commandAllocator.Get()));
    return cmdList;
}

void D3D::ExecuteCommandList(ID3D12GraphicsCommandList* cmdList)
{
    ID3D12CommandAllocator* commandAllocator;
    UINT dataSize = sizeof(commandAllocator);

    V(cmdList->GetPrivateData(__uuidof(ID3D12CommandAllocator), &dataSize, &commandAllocator));

    ID3D12CommandList* ppCommandLists[] = { cmdList };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    const UINT64 fence = m_fenceValue;
    V(m_commandQueue->Signal(m_fence.Get(), fence));
    m_fenceValue++;
    m_commandAllocatorQueue.push({fence,commandAllocator});
    m_commandListQueue.push(cmdList);
    commandAllocator->Release();
}

void D3D::Present()
{
    V(m_swapChain->Present(1, 0));
    const UINT64 fence = m_fenceValue;
    V(m_commandQueue->Signal(m_fence.Get(), fence));
    m_frameBufferFences[m_frameIndex] = fence;
    m_fenceValue++;

    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    WaitForSignal(m_frameBufferFences[m_frameIndex]);
}

void D3D::Flush()
{
    // Signal and increment the fence value.
    const UINT64 fence = m_fenceValue;
    V(m_commandQueue->Signal(m_fence.Get(), fence));
    m_fenceValue++;

    // Wait until the previous frame is finished.
    if (m_fence->GetCompletedValue() < fence)
    {
        V(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }
}

void D3D::WaitForSignal(UINT64 fence) const
{
    if (m_fence->GetCompletedValue() < fence)
    {
        V(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }
}

bool D3D::IsFenceComplete(const UINT64 fenceVal) const
{
    return m_fence->GetCompletedValue() >= fenceVal;
}
