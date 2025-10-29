//
// Created by fionaw on 28/10/2025.
//

#ifndef CHERRYPIP_PATHTRACINGCONTEXT_H
#define CHERRYPIP_PATHTRACINGCONTEXT_H
#include <d3d12.h>
#include <memory>
#include <random>
#include <vector>
#include <wrl/client.h>

#include "Camera.h"
#include "HWI/D12Resource.h"
#include "HWI/Heap.h"
#include "HWI/Model.h"
#include "HWI/Texture.h"

struct PtConfig;
class RootSig;
class Shader;
class Material;
using Microsoft::WRL::ComPtr;

#include "PTBuffers.h"

class BLAS;
class TLAS;

#define INITIAL_SEED 1887

class PathTracingContext
{
public:
    void Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const std::shared_ptr<TLAS>& tlas,
              const std::vector<std::shared_ptr<BLAS>>& blasList, Heap* heap, D12Resource* rtv);
    void FillMaterial(ID3D12Device* device, Material* material, Heap* heap) const;
    void Render(ID3D12GraphicsCommandList* cmdList, ID3D12RootSignature* rootSig, D12Resource* rtv,
                ID3D12PipelineState* pso,
                const Camera* camera, const Material* material, const XMMATRIX& projMatrix, const PtConfig& config);
    void Reset();

    std::shared_ptr<D12Resource> GetInstanceDataBuffer() const { return m_instanceDataBuffer; }
    UINT GetNumInstances() const { return m_blasList.size(); }
    static size_t GetInstanceDataSize() { return sizeof(PtInstanceData); }

private:
    std::shared_ptr<TLAS> m_tlas;
    std::vector<std::shared_ptr<BLAS>> m_blasList;

    uint32_t m_numFrames = 0;

    Model m_fullScreenTriangle;
    std::shared_ptr<Texture> m_pathTraceOutTexture, m_accumTexture;

    std::shared_ptr<Shader> m_accumShader;
    std::shared_ptr<RootSig> m_accumRootSig;
    std::shared_ptr<Material> m_accumMaterial;

    std::vector<PtInstanceData> m_instanceDataList;
    std::shared_ptr<D12Resource> m_instanceDataBuffer, m_vertexMegaBuffer, m_indexMegaBuffer, m_materialBuffer;
    UINT m_vertexMegaBufferCount = 0, m_indexMegaBufferCount = 0;

    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;

    std::mt19937 m_rng;
    std::uniform_int_distribution<UINT> m_rngDist;
    UINT m_curRngState = INITIAL_SEED;
};


#endif //CHERRYPIP_PATHTRACINGCONTEXT_H
