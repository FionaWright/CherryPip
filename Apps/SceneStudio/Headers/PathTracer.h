#ifndef H_PATH_TRACER_H
#define H_PATH_TRACER_H

class D3D;

class PathTracer
{
public:
    void Init(D3D* d3d);
    void BuildScene(D3D* d3d, ID3D12GraphicsCommandList* cmdList, EnvMap* envMap, Scene* scene, Heap* heap);

    void Update(D3D* d3d);
    void PostUpdate(D3D* d3d);

    void Render(D3D* d3d, ID3D12GraphicsCommandList* cmdList);
    void RenderLines(D3D* d3d, ID3D12GraphicsCommandList* cmdList, const XMMATRIX& vpMatrix);
    void RenderGUI();

    ReadbackManager* GetReadbackManager() const { return &m_readbackManager; }
    PtConfig& GetConfig() const { return m_config; }

    void Reset();

private:
    void compilePtShader(const D3D* d3d, bool isSpectral);

    PathTracingContext m_ptContext;

    std::vector<D3D12_INPUT_ELEMENT_DESC> m_shaderILD;
    std::shared_ptr<Shader> m_shader;
    std::shared_ptr<RootSig> m_rootSig;

    PtConfig m_config = {};

#ifdef _DEBUG
    PathVisualizer m_pathVisualizer;
    bool m_takePathVisualizationSnapshot = false;
    bool m_completedPathVisualizationSnapshot = false;
    std::vector<std::shared_ptr<DebugLine>> m_pathVisualizationLines;

    ReadbackManager m_readbackManager;
    RmseTester m_rmseTester;
    uint32_t m_rmseTesterSlot = 0;
#endif
}

#endif