#ifndef H_PATH_TRACER_H
#define H_PATH_TRACER_H

#ifdef _DEBUG
#include "ReadbackManager.h"
#include "Debug/DebugLine.h"
#include "Debug/PathVisualizer.h"
#endif

#include "Render/EnvMap.h"
#include "Render/PathTracingContext.h"
#include "Render/Scene.h"

struct DirLightConfig;
class D3D;

struct SpectralConfig
{
    bool SingleLambdaRendering = true; // Turn to enum when I introduce hero sampling
    bool DebugForceWavelengthEnabled = false;
    float DebugForcedWavelength = 500.0f;
};

class PathTracer
{
public:
    void Init(D3D* d3d, ID3D12GraphicsCommandList* cmdList, Heap* heap, TextureRTV* rtvForReadback);
    void BuildScene(D3D* d3d, ID3D12GraphicsCommandList* cmdList, EnvMap* envMap, Scene* scene, Heap* heap);

    void Update(D3D* d3d, Heap* heap, bool shaderDirty, bool isSpectral, bool envMapEnabled);
    void PostUpdate(D3D* d3d, Heap* heap, const std::shared_ptr<Shader>& shaderLine);

    void Render(D3D* d3d, ID3D12GraphicsCommandList* cmdList, Heap* heap, const Camera* camera,
                const XMMATRIX& projMatrix, CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle, TextureRTV* rtvTex,
                XMFLOAT2 mousePosOnClick, const DirLightConfig& dirLightConfig);
    void RenderLines(D3D* d3d, ID3D12GraphicsCommandList* cmdList, const XMMATRIX& vpMatrix);
    void RenderGUI(bool& outPtDirty, bool& outShaderDirty, bool& outSceneDirty, XMFLOAT2 mousePosOnClick, bool isSpectral);

    ReadbackManager* GetReadbackManager() { return &m_readbackManager; }
    PtConfig& GetConfig() { return m_config; }
    PathTracingContext& GetContext() { return m_ptContext; }

    void Reset();

private:
    void compilePtShader(const D3D* d3d, bool isSpectral, bool envMapEnabled);

    PathTracingContext m_ptContext;

    std::vector<D3D12_INPUT_ELEMENT_DESC> m_shaderILD;
    std::shared_ptr<Shader> m_shader;
    std::shared_ptr<RootSig> m_rootSig;

    PtConfig m_config = {};
    SpectralConfig m_spectralConfig = {};

#ifdef _DEBUG
    PathVisualizer m_pathVisualizer;
    bool m_takePathVisualizationSnapshot = false;
    bool m_completedPathVisualizationSnapshot = false;
    std::vector<std::shared_ptr<DebugLine>> m_pathVisualizationLines;

    ReadbackManager m_readbackManager;
#endif
};

#endif
