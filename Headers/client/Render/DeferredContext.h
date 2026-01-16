//
// Created by fiona on 15/01/2026.
//

#ifndef CHERRYPIP_DEFERREDCONTEXT_H
#define CHERRYPIP_DEFERREDCONTEXT_H
#include <d3d12.h>

#include "Scene.h"
#include "TextureRTV.h"
#include "HWI/Shader.h"


class Skybox;

class DeferredContext
{
public:
    void Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, Heap* heap);
    void SetScene(Scene* scene);

    void Render(const D3D* d3d, ID3D12GraphicsCommandList* cmdList, const XMMATRIX& vMatrix, const XMMATRIX& pMatrix, const Skybox* skybox);

    D12Resource* GetAlbedo() { return m_rtvAlbedo.GetD12Resource(); }
    D12Resource* GetNormalsDepth() { return m_rtvNormalsDepth.GetD12Resource(); }

private:
    TextureRTV m_rtvAlbedo, m_rtvNormalsDepth;
    std::vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> m_rtvHandles;
    Scene* m_scene = nullptr;

    RootSig m_rootSigGBuffer;
    Shader m_shaderGBuffer; // Bad system, requires matching root sig between forward/deferred
};


#endif //CHERRYPIP_DEFERREDCONTEXT_H