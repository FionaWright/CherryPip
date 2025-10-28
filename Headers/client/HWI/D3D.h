//
// Created by fionaw on 22/09/2025.
//

#ifndef PT_D3D_H
#define PT_D3D_H

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// Windows Runtime Library. Needed for ComPtr<> template class.
#include <wrl.h>

// DirectX 12 specific headers.
#include <d3d12.h>
#include <dxgi1_5.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <fstream>
#include <queue>

using namespace DirectX;

// D3D12 extension library.
#include "d3dx12.h"

using Microsoft::WRL::ComPtr;

class D3D
{
public:
    ~D3D();
    void Init(size_t width, size_t height);
    ComPtr<ID3D12GraphicsCommandList> CreateCmdList(ID3D12CommandAllocator* allocator) const;

    ID3D12Device* GetDevice() const { return m_device.Get(); }
    UINT GetFrameIndex() const { return m_frameIndex; }

    ID3D12Resource* GetCurrRTV() const { return m_renderTargets[m_frameIndex].Get(); }
    ID3D12Resource* GetCurrDSV() const { return m_depthStencilBuffer[m_frameIndex].Get(); }
    D3D12_CPU_DESCRIPTOR_HANDLE GetRtvHeapStart() const { return m_rtvHeap->GetCPUDescriptorHandleForHeapStart(); }
    D3D12_CPU_DESCRIPTOR_HANDLE GetDsvHeapStart() const { return m_dsvHeap->GetCPUDescriptorHandleForHeapStart(); }
    UINT GetRtvDescriptorSize() const { return m_rtvDescriptorSize;}
    UINT GetDsvDescriptorSize() const { return m_dsvDescriptorSize;}

    bool GetRayTracingSupported() const { return m_rayTracingSupported; }

    ComPtr<ID3D12CommandAllocator> CreateAllocator(D3D12_COMMAND_LIST_TYPE type) const;
    ComPtr<ID3D12GraphicsCommandList> GetAvailableCmdList(D3D12_COMMAND_LIST_TYPE type);
    void ExecuteCommandList(ID3D12GraphicsCommandList* cmdList);
    void Present();
    void Flush();
    void WaitForSignal(UINT64 fence) const;
    bool IsFenceComplete(UINT64 fenceVal) const;

private:

    static constexpr UINT c_FrameCount = 3;

    // Pipeline objects.
    ComPtr<IDXGISwapChain3> m_swapChain;
    ComPtr<ID3D12Device> m_device;
    ComPtr<ID3D12Resource> m_renderTargets[c_FrameCount];
    ComPtr<ID3D12Resource> m_depthStencilBuffer[c_FrameCount];
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap, m_dsvHeap;
    UINT m_rtvDescriptorSize = 0, m_dsvDescriptorSize = 0;

    struct CommandAllocatorEntry
    {
        UINT64 Fence;
        ComPtr<ID3D12CommandAllocator> Allocator;
    };

    std::queue<CommandAllocatorEntry> m_commandAllocatorQueue;
    std::queue<ComPtr<ID3D12GraphicsCommandList>> m_commandListQueue;

    ComPtr<ID3D12CommandQueue> m_commandQueue;

    bool m_useWarpDevice = false;
    bool m_tearingSupport = false;
    bool m_rayTracingSupported = false;

    // Synchronization objects.
    UINT m_frameIndex = 0;
    HANDLE m_fenceEvent = {};
    ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValue = 0;
    UINT64 m_frameBufferFences[c_FrameCount] = {};

    // Debugging
    ComPtr<ID3D12InfoQueue1> m_infoQueue;
    std::ofstream m_logFile;
};

#endif //PT_D3D_H