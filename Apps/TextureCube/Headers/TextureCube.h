#ifndef PT_TEXTURECUBE_H
#define PT_TEXTURECUBE_H

#include "Apps/App.h"
#include "HWI/RootSig.h"
#include "HWI/Shader.h"
#include "HWI/Texture.h"
#include "Render/CameraController.h"
#include "Render/Transform.h"

using Microsoft::WRL::ComPtr;

class TextureCube : public App
{
public:
    TextureCube();
    void OnInit(D3D* d3d) override;
    void OnUpdate(D3D* d3d) override;

    const WCHAR* GetTitle() const   { return m_title.c_str(); }

    void ParseCommandLineArgs(_In_reads_(argc) WCHAR* argv[], int argc) override;

    float m_AspectRatio;

private:
    static constexpr UINT c_FrameCount = 3;

    void setCustomWindowText(LPCWSTR text) const;

    // App resources.
    ComPtr<ID3D12Resource> m_vertexBuffer;
    UINT m_vertexCount = 0;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

    ComPtr<ID3D12DescriptorHeap> m_cbvSrvUavHeap;
    UINT m_descriptorIncSize = 0;
    ComPtr<ID3D12Resource> m_cbv;

    Shader m_shaderNormals;
    RootSig m_rootSig;
    CameraController m_camera;
    Transform m_transformCube;
    Texture m_tex;

    // Window title.
    std::wstring m_title;
    
    void loadAssets(D3D* d3d);
    void populateCommandList(D3D* d3d, ID3D12GraphicsCommandList* cmdList);
};


#endif //PT_TEXTURECUBE_H