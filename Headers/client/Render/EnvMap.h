//
// Created by fionaw on 14/11/2025.
//

#ifndef CHERRYPIP_ENVMAP_H
#define CHERRYPIP_ENVMAP_H
#include "HWI/Material.h"
#include "HWI/RootSig.h"
#include "HWI/Shader.h"
#include "HWI/Texture.h"


class EnvMap
{
public:
    void CreateCubemapResource(ID3D12Device* device);
    void Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const std::wstring& filePath, float rotation, Heap* heap);
    void InitCubemap(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, Heap* heap);

    XMFLOAT3 GetDirectionOfHighestIntensity(D3D* d3d, Heap* heap);

    D12Resource* GetPano() { return m_pano.GetD12Resource(); }
    D12Resource* GetEA() { return m_ea.GetD12Resource(); }
    D12Resource* GetCubemap() { return m_cubemap.GetD12Resource(); }

private:
    void initResources(ID3D12Device* device);

    Texture m_pano, m_ea, m_cubemap;
    float m_rotation = 0.0f;
    std::wstring m_currentPanoFilepath = L"";
    bool m_resourcesInitialized = false;

    RootSig m_rootSigPanoToEA, m_rootSigPanoToCM;
    Shader m_shaderPanoToEA, m_shaderPanoToCM;
    Material m_matPanoToEA, m_matPanoToCM;

    RootSig m_rootSigMaxLumRedSearch;
    Shader m_shaderMaxLumRedSearch;
    Material m_matMaxLumRedSearch;
    D12Resource m_bufferMaxLumRedSearch;
    D12Resource m_readbackBufferMaxLumRedSearch;
};


#endif //CHERRYPIP_ENVMAP_H