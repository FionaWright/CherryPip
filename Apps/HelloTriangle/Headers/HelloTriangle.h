#ifndef PT_HELLOTRIANGLE_H
#define PT_HELLOTRIANGLE_H

#include "Apps/App.h"
#include "HWI/Shader.h"

using Microsoft::WRL::ComPtr;

class HelloTriangle : public App
{
public:
    HelloTriangle();
    void OnInit(D3D* d3d) override;
    void OnUpdate(D3D* d3d) override;

    void OnKeyDown(UINT8 /*key*/)   {}
    void OnKeyUp(UINT8 /*key*/)     {}

    const WCHAR* GetTitle() const { return m_title.c_str(); }

    void ParseCommandLineArgs(_In_reads_(argc) WCHAR* argv[], int argc) override;

    float m_AspectRatio;

private:
    static constexpr UINT c_FrameCount = 2;

    void setCustomWindowText(LPCWSTR text) const;

    // App resources.
    ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
    ComPtr<ID3D12RootSignature> m_rootSignature;

    Shader m_shader;

    // Window title.
    std::wstring m_title;
    
    void loadAssets(ID3D12Device* device);
    void populateCommandList(const D3D* d3d, ID3D12GraphicsCommandList* cmdList) const;
};


#endif //PT_HELLOTRIANGLE_H
