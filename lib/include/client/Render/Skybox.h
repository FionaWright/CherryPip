//
// Created by fiona on 14/01/2026.
//

#ifndef CHERRYPIP_SKYBOX_H
#define CHERRYPIP_SKYBOX_H

#include "CBV.h"
#include "HWI/Material.h"
#include "HWI/Model.h"
#include "HWI/RootSig.h"
#include "HWI/Shader.h"

class Skybox
{
public:
    void Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, Heap* heap, D12Resource* cubemap);
    void RenderForward(const D3D* d3d, ID3D12GraphicsCommandList* cmdList, const XMMATRIX& vMatrix, const XMMATRIX& pMatrix) const;

    void UpdateCubemap(ID3D12Device* device, D12Resource* cubemap);
    void GenerateIrradianceMap(ID3D12GraphicsCommandList* cmdList, const Heap* heap);

    D12Resource* GetIrradianceMap() { return m_texIrradianceIBL.GetD12Resource(); }

private:
    RootSig m_rootSig;
    Shader m_shaderForward;
    Material m_matForwardRender;
    Model m_cube;

    RootSig m_rootSigGenIrr;
    Shader m_shaderGenIrr;
    Material m_matGenIrr;
    Texture m_texIrradianceIBL;

    Heap* m_pHeap = nullptr;
};


#endif //CHERRYPIP_SKYBOX_H