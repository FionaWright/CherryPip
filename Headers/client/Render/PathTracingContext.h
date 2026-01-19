//
// Created by fionaw on 28/10/2025.
//

#ifndef CHERRYPIP_PATHTRACINGCONTEXT_H
#define CHERRYPIP_PATHTRACINGCONTEXT_H

#include "Camera.h"
#include "CBV.h"
#include "HWI/D12Resource.h"
#include "HWI/Heap.h"
#include "HWI/Model.h"
#include "HWI/Texture.h"

class EnvMap;
class Scene;
struct PtConfig;
class RootSig;
class Shader;
class Material;
using Microsoft::WRL::ComPtr;

#include "PTBuffers.h"

class BLAS;
class TLAS;

enum PathTracerMode : uint32_t
{
    eLambertDiff,
    eGlossy,
    eGlass,
    eGgxSmithMicrofacet,
    eFurnaceTestClassic,
    eFurnaceTestEmissive
};

struct PtConfig
{
    uint32_t SPP = 1;
    uint32_t NumBounces = 8;
    uint32_t MaxFrameNum = 0;
    uint32_t RussianRouletteMinBounces = 4;
    PathTracerMode Mode = eGgxSmithMicrofacet;
    DebugBuffer DebugBufferIdx = DebugBuffer::eNormalsShaded;
    float DirLightCosAngularRadius = 0.00465f;
    alignas(4) bool DebugMode = false;
    alignas(4) bool AccumulationEnabled = true;
    alignas(4) bool JitterEnabled = true;
    alignas(4) bool ReadbackEnabled = false;
    alignas(4) bool ReadbackEveryFrame = false;
    alignas(4) bool EnvMapIsEqualArea = true;
    alignas(4) bool RussianRouletteEnabled = true;
    alignas(4) bool DirLightEnabled = true;
    alignas(4) bool NormalMapsEnabled = true;
};

class PathTracingContext
{
public:
    void Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, Heap* heap);
    void BuildScene(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const Scene* scene, Heap* heap, D12Resource* envMap);
    void Render(ID3D12GraphicsCommandList* cmdList, ID3D12RootSignature* rootSig,
                ID3D12PipelineState* pso,
                const Camera* camera, Heap* heap, const XMMATRIX& projMatrix, const PtConfig& config, float dirLightIntensity, XMFLOAT3 dirLightColor, XMFLOAT3 dirLightDir,
                int debugModeIdx = -1);
    void Reset();

    uint32_t GetFrameNum() const { return m_numFrames; }
    Texture* GetAccumTexture() const { return m_accumTexture.get(); }

    std::shared_ptr<D12Resource> GetInstanceDataBuffer() const { return m_instanceDataBuffer; }
    UINT GetNumInstances() const { return m_blasList.size(); }
    static size_t GetInstanceDataSize() { return sizeof(PtInstanceData); }

private:
    std::shared_ptr<TLAS> m_tlas;
    std::vector<std::shared_ptr<BLAS>> m_blasList;

    std::shared_ptr<Material> m_material;

    uint32_t m_numFrames = 0;

    Model m_fullScreenTriangle;
    std::shared_ptr<Texture> m_accumTexture, m_accumClearBuffer;

    std::vector<PtInstanceData> m_instanceDataList;
    std::shared_ptr<D12Resource> m_instanceDataBuffer, m_vertexMegaBuffer, m_indexMegaBuffer, m_materialBuffer;
    UINT m_vertexMegaBufferCount = 0, m_indexMegaBufferCount = 0;
};


#endif //CHERRYPIP_PATHTRACINGCONTEXT_H
