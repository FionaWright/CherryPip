//
// Created by fiona on 15/01/2026.
//

#ifndef CHERRYPIP_DEFERREDCONTEXT_H
#define CHERRYPIP_DEFERREDCONTEXT_H


#include "CBV.h"
#include "Scene.h"
#include "TextureRTV.h"
#include "HWI/Shader.h"


class DenoisingManager;
class Skybox;

class DeferredContext
{
public:
    void Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, Heap* heapRTV, Heap* heap);
    void SetScene(Scene* scene);

    bool IsInitialized() const { return m_initialized; }

    void RenderGBuffer(const D3D* d3d, ID3D12GraphicsCommandList* cmdList, const XMMATRIX& vMatrix,
                       const XMMATRIX& pMatrix, const Skybox* skybox);

    void RenderLighting(const D3D* d3d, ID3D12GraphicsCommandList* cmdList, Heap* heap, const XMMATRIX& pMatrix,
                        TextureRTV* output, const XMFLOAT3& dirLightDir, D12Resource* skybox,
                        D12Resource* irradianceMap, D12Resource* brdfIntegrationMap,
                        RasterDebugMode debugMode, DenoisingManager* denoisingManager);

    D12Resource* GetAlbedo() { return m_rtvAlbedo.GetD12Resource(); }

    D12Resource* GetNormalsDepth() { return m_rtvNormalsDepth.GetD12Resource(); }

private:
    TextureRTV m_rtvAlbedo, m_rtvNormalsDepth, m_rtvRoughMet, m_rtvEmissive, m_rtvScratch;
    std::vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> m_rtvHandles;
    Scene* m_scene = nullptr;
    bool m_initialized = false;

    RootSig m_rootSigGBuffer;
    Shader m_shaderGBuffer; // Bad system, requires matching root sig between forward/deferred

    RootSig m_rootSigLighting;
    Shader m_shaderLighting;
    Material m_matLighting;
    Model m_fullScreenTriangle;
};


#endif //CHERRYPIP_DEFERREDCONTEXT_H
