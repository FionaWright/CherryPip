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

enum MicrofacetNdfType : uint32_t
{
    eGGX,
    eBeckmann
};

enum MicrofacetMaskingType : uint32_t
{
    eSmith,
    eVCavity
};

enum PathTracerLightingModel : uint32_t
{
    eLambertDiff,
    eGlossy,
    eMicrofacet
};

enum PathTracerSamplingStrategy : uint32_t
{
    eIndependent,
    eHalton,
    eHaltonApple,
    eHaltonOwen,
};

struct PtConfig
{
    uint32_t SPP = 1;
    uint32_t NumBounces = 8;
    uint32_t MaxFrameNum = 0;
    uint32_t RussianRouletteMinBounces = 4;
    PathTracerLightingModel LightingModel = eMicrofacet;
    MicrofacetNdfType NdfType = eGGX;
    MicrofacetMaskingType MaskingType = eSmith;
    DebugInfoOutput DebugInfoOutputMode = DebugInfoOutput::eNormalsShaded;
    PathTracerSamplingStrategy SamplingStrat = eIndependent;
    float DirLightCosAngularRadius = 0.00465f;
    float FireflyThreshold = 10.0f;
    float DofFocalDist = 5.0f;
    float DofLensRadius = 0.05f;
    alignas(4) bool DebugInfoOutputEnabled = false;
    alignas(4) bool DebugForceSpecular = false;
    alignas(4) bool DebugForceDiffuse = false;
    alignas(4) bool DebugPathVisualization = false;
    alignas(4) bool AccumulationEnabled = true;
    alignas(4) bool JitterEnabled = true;
    alignas(4) bool ReadbackEnabled = false;
    alignas(4) bool ReadbackEveryFrame = false;
    alignas(4) bool EnvMapIsEqualArea = true;
    alignas(4) bool RussianRouletteEnabled = true;
    alignas(4) bool DirLightEnabled = true;
    alignas(4) bool DirLightIsDistant = true;
    alignas(4) bool NormalMapsEnabled = true;
    alignas(4) bool SampleVisibleNormals = false;
    alignas(4) bool AnisotropyEnabled = false;
    alignas(4) bool DepthOfFieldEnabled = false;
    alignas(4) bool FurnaceTestHdReflect = false;
    alignas(4) bool FurnaceTestHhEmit = false;
    alignas(4) bool AlphaTestingEnabled = true;
    alignas(4) bool ImportanceSamplingEnabled = true;
    alignas(4) bool GlassModelEnabled = false;
    alignas(4) bool GammaCorrection = true;
};

class PathTracingContext
{
public:
    void Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, Heap* heap);
    void BuildScene(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const Scene* scene, Heap* heap, D12Resource* envMap);
    void Render(ID3D12GraphicsCommandList* cmdList, ID3D12RootSignature* rootSig,
                ID3D12PipelineState* pso,
                const Camera* camera, Heap* heap, const XMMATRIX& projMatrix,
                const PtConfig& config,
                float dirLightIntensity, XMFLOAT3 dirLightColor, XMFLOAT3 dirLightDir,
                int debugModeIdx = -1, bool takingPathVisSnapshot = false, XMFLOAT2 pathVisSelectedPixel = {});
    void Reset();

    uint32_t GetFrameNum() const { return m_frameIdx; }
    Texture* GetAccumTexture() const { return m_accumTexture.get(); }

    std::shared_ptr<D12Resource> GetInstanceDataBuffer() const { return m_instanceDataBuffer; }
    UINT GetNumInstances() const { return m_blasList.size(); }
    static size_t GetInstanceDataSize() { return sizeof(PtInstanceData); }

#ifdef _DEBUG
    void SetMaterialPathVisualizationBuffer(ID3D12Device* device, Heap* heap, const D12Resource* buffer, uint32_t numElements);
#endif

private:
    std::shared_ptr<TLAS> m_tlas;
    std::vector<std::shared_ptr<BLAS>> m_blasList;

    std::shared_ptr<Material> m_material;

    uint32_t m_frameIdx = 0;

    Model m_fullScreenTriangle;
    std::shared_ptr<Texture> m_accumTexture, m_accumClearBuffer;

    std::vector<PtInstanceData> m_instanceDataList;
    std::shared_ptr<D12Resource> m_instanceDataBuffer, m_vertexMegaBuffer, m_indexMegaBuffer, m_materialBuffer;
    UINT m_vertexMegaBufferCount = 0, m_indexMegaBufferCount = 0;

    CbvPrimes m_cbvPrimes{};
    bool m_primesInitialized = false;

#ifdef _DEBUG
    bool m_setPathVisualizationBuffer = false;
#endif
};


#endif //CHERRYPIP_PATHTRACINGCONTEXT_H
