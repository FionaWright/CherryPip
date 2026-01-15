//
// Created by fiona on 15/01/2026.
//

#ifndef CHERRYPIP_DENOISINGMANAGER_H
#define CHERRYPIP_DENOISINGMANAGER_H
#include <d3d12.h>

#include "TextureRTV.h"
#include "HWI/D12Resource.h"
#include "HWI/Material.h"
#include "HWI/Model.h"
#include "HWI/RootSig.h"
#include "HWI/Shader.h"

class DenoisingManager
{
public:
    void Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, Heap* heap, D12Resource* pp1, D12Resource* pp2);

    TextureRTV* DenoiseBox(ID3D12GraphicsCommandList* cmdList, TextureRTV* pp1, TextureRTV* pp2, uint32_t radius) const;
    TextureRTV* DenoiseGauss(ID3D12GraphicsCommandList* cmdList, TextureRTV* pp1, TextureRTV* pp2, uint32_t radius) const;

private:
    void initBox(ID3D12Device* device, Heap* heap, D12Resource* pp1);
    void initGauss(ID3D12Device* device, Heap* heap, D12Resource* pp1, D12Resource* pp2);

    Model m_fullScreenTriangle;
    Heap* m_pHeap = nullptr;
    D3D12_INPUT_LAYOUT_DESC m_ild = {};
    std::vector<D3D12_INPUT_ELEMENT_DESC> m_ildDesc = {};

    RootSig m_rootSigBox;
    Shader m_shaderBox;
    Material m_matBox;

    RootSig m_rootSigGauss;
    Shader m_shaderGaussH,  m_shaderGaussV;
    Material m_matGaussH, m_matGaussV;
    TextureRTV m_rtvIntermediate;
};


#endif //CHERRYPIP_DENOISINGMANAGER_H