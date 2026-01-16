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

#define MAX_ATROUS_ITERATIONS 10

enum DenoisingType : uint32_t
{
    eBox,
    eGaussian,
    eMedian,
    eATrous,
    eNRD
};

class DenoisingManager
{
public:
    void Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, Heap* heap, D12Resource* pp1, D12Resource* pp2,
              D12Resource* normalsDepth);

    TextureRTV* DenoiseBox(ID3D12GraphicsCommandList* cmdList, TextureRTV* pp1, TextureRTV* pp2, uint32_t radius) const;
    TextureRTV* DenoiseGauss(ID3D12GraphicsCommandList* cmdList, TextureRTV* pp1, TextureRTV* pp2,
                             uint32_t radius) const;
    TextureRTV* DenoiseMedian(ID3D12GraphicsCommandList* cmdList, TextureRTV* pp1, TextureRTV* pp2) const;
    TextureRTV* DenoiseATrous(ID3D12GraphicsCommandList* cmdList, const XMMATRIX& vMatrix, const XMMATRIX& pMatrix,
                              TextureRTV* pp1, TextureRTV* pp2, uint32_t iterations,
                              float phiC, float phiN, float phiP) const;

private:
    void initBox(ID3D12Device* device, Heap* heap, D12Resource* pp1);
    void initGauss(ID3D12Device* device, Heap* heap, D12Resource* pp1, D12Resource* pp2);
    void initMedian(ID3D12Device* device, Heap* heap, D12Resource* pp1, D12Resource* pp2);
    void initATrous(ID3D12Device* device, Heap* heap, D12Resource* pp1, D12Resource* pp2, D12Resource* normalsDepth);

    Model m_fullScreenTriangle;
    Heap* m_pHeap = nullptr;
    D3D12_INPUT_LAYOUT_DESC m_ild = {};
    D3D12_STATIC_SAMPLER_DESC m_sampler = {};
    std::vector<D3D12_INPUT_ELEMENT_DESC> m_ildDesc = {};

    RootSig m_rootSigBox;
    Shader m_shaderBox;
    Material m_matBox;

    RootSig m_rootSigGauss;
    Shader m_shaderGaussH, m_shaderGaussV;
    Material m_matGaussH, m_matGaussV;

    RootSig m_rootSigMedian;
    Shader m_shaderMedian;
    Material m_matMedian;

    RootSig m_rootSigATrous;
    Shader m_shaderATrous;
    std::vector<Material> m_matsATrous; // Inefficient, should use root constants but no support yet
};


#endif //CHERRYPIP_DENOISINGMANAGER_H
