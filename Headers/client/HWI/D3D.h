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

using namespace DirectX;

// D3D12 extension library.
#include "d3dx12.h"

using Microsoft::WRL::ComPtr;

class D3D
{
public:
    void Init(size_t width, size_t height);

    ID3D12Device* GetDevice() const { return m_device.Get(); }

    ComPtr<ID3D12GraphicsCommandList> GetNewCommandList() const;
    ID3D12Resource* GetCurrRTV() const { return m_renderTargets[m_frameIndex].Get(); }
    D3D12_CPU_DESCRIPTOR_HANDLE GetRtvHeapStart() const { return m_rtvHeap->GetCPUDescriptorHandleForHeapStart(); }
    UINT GetFrameIndex() const { return m_frameIndex; }
    UINT GetRtvDescriptorSize() const { return m_rtvDescriptorSize;}

    void ExecuteCommandList(ID3D12GraphicsCommandList* cmdList) const;
    void Present();
    void Flush();
    void WaitForSignal(UINT64 fence) const;

private:

    static constexpr UINT c_FrameCount = 3;

    // Pipeline objects.
    ComPtr<IDXGISwapChain3> m_swapChain;
    ComPtr<ID3D12Device> m_device;
    ComPtr<ID3D12Resource> m_renderTargets[c_FrameCount];
    ComPtr<ID3D12CommandAllocator> m_commandAllocator;
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12InfoQueue1> m_infoQueue;
    UINT m_rtvDescriptorSize = 0;

    bool m_useWarpDevice = false;

    // Synchronization objects.
    UINT m_frameIndex = 0;
    HANDLE m_fenceEvent = {};
    ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValue = 0;
    UINT64 m_frameBufferFences[c_FrameCount] = {};
};

#endif //PT_D3D_H