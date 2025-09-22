#ifndef PT_HELLOTRIANGLE_H
#define PT_HELLOTRIANGLE_H

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

#include "Helper.h"

#define WIDTH 600
#define HEIGHT 400

using Microsoft::WRL::ComPtr;

class HelloTriangle
{
public:
    HelloTriangle();
    void OnInit();
    void OnUpdate();

    // Samples override the event handlers to handle specific messages.
    virtual void OnKeyDown(UINT8 /*key*/)   {}
    virtual void OnKeyUp(UINT8 /*key*/)     {}

    // Accessors.
    UINT GetWidth() const           { return m_Width; }
    UINT GetHeight() const          { return m_Height; }
    const WCHAR* GetTitle() const   { return m_title.c_str(); }

    void ParseCommandLineArgs(_In_reads_(argc) WCHAR* argv[], int argc);

    UINT m_Width;
    UINT m_Height;
    float m_AspectRatio;

    // Adapter info.
    bool m_UseWarpDevice;

private:
    static constexpr UINT c_FrameCount = 2;

    struct Vertex
    {
        XMFLOAT3 position;
        XMFLOAT4 color;
    };

    std::wstring getAssetFullPath(LPCWSTR assetName) const;

    void getHardwareAdapter(
        _In_ IDXGIFactory1* pFactory,
        _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter,
        bool requestHighPerformanceAdapter = false);

    void setCustomWindowText(LPCWSTR text);

    // Pipeline objects.
    CD3DX12_VIEWPORT m_viewport;
    CD3DX12_RECT m_scissorRect;
    ComPtr<IDXGISwapChain3> m_swapChain;
    ComPtr<ID3D12Device> m_device;
    ComPtr<ID3D12Resource> m_renderTargets[c_FrameCount];
    ComPtr<ID3D12CommandAllocator> m_commandAllocator;
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12PipelineState> m_pipelineState;
    ComPtr<ID3D12GraphicsCommandList> m_commandList;
    UINT m_rtvDescriptorSize;

    // App resources.
    ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

    // Synchronization objects.
    UINT m_frameIndex;
    HANDLE m_fenceEvent;
    ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValue;

    // Root assets path.
    std::wstring m_assetsPath;

    // Window title.
    std::wstring m_title;

    void loadPipeline();
    void loadAssets();
    void populateCommandList();
    void waitForPreviousFrame();
};


#endif //PT_HELLOTRIANGLE_H
