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

class D3D;

// D3D12 extension library.
#include "d3dx12.h"

#define WIDTH 600
#define HEIGHT 400

using Microsoft::WRL::ComPtr;

class SpinningCube
{
public:
    SpinningCube();
    void OnInit(D3D* d3d);
    void OnUpdate(D3D* d3d);

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

    static ID3D12Device* s_device;

private:
    static constexpr UINT c_FrameCount = 2;

    struct Vertex
    {
        XMFLOAT3 position;
        XMFLOAT4 color;
    };

    std::wstring getAssetFullPath(LPCWSTR assetName) const;

    void setCustomWindowText(LPCWSTR text);

    // App resources.
    ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12PipelineState> m_pipelineState;

    // Root assets path.
    std::wstring m_assetsPath;

    // Window title.
    std::wstring m_title;
    
    void loadAssets(ID3D12Device* device);
    void populateCommandList(D3D* d3d, ID3D12GraphicsCommandList* cmdList);
};


#endif //PT_HELLOTRIANGLE_H
