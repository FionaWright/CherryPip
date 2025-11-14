#ifndef PT_SCENESTUDIO_H
#define PT_SCENESTUDIO_H

#include "CBV.h"
#include "System/App.h"
#include "HWI/Heap.h"
#include "HWI/RootSig.h"
#include "HWI/Shader.h"
#include "HWI/Texture.h"
#include "HWI/TLAS.h"
#include "Render/CameraController.h"
#include "Render/EnvMap.h"
#include "Render/Object.h"
#include "Render/PathTracingContext.h"
#include "Render/RasterContext.h"
#include "Render/TextureRTV.h"
#include "Render/Transform.h"

#ifdef _DEBUG
#include "ReadbackManager.h"
#endif

class BLAS;
using Microsoft::WRL::ComPtr;

struct RasterConfig
{
    RasterDebugMode Mode = eNormals;
    XMFLOAT3 DirLighting = XMFLOAT3(1, 0, 1);
};

enum RenderBackend : uint32_t
{
    eForward,
    ePathTracer,
    MAX_COUNT,
};

struct StudioConfig
{
    RenderBackend Backend = ePathTracer;
    PtConfig PT = {};
    RasterConfig Raster = {};
};

struct SceneConfig
{
    std::string Name;
    std::wstring GltfPath;
    Transform Transform;
    XMFLOAT3 InitialCamPos;
    XMFLOAT2 InitialCamPitchYaw;

    int SceneIdx = -1;
};

class SceneStudio final : public App
{
public:
    void OnInit(D3D* d3d) override;
    void OnUpdate(D3D* d3d, ID3D12GraphicsCommandList* cmdList) override;
    void RenderGUI() override;

    [[nodiscard]] const char* GetName() const override { return "Scene Studio"; }

    float m_AspectRatio = 0;

private:
    void loadAssets(D3D* d3d);
    void loadRasterAssets(const D3D* d3d);
    void initScene(D3D* d3d, ID3D12GraphicsCommandList* cmdList, uint32_t configIdx);
    void renderPathTracer(D3D* d3d, ID3D12GraphicsCommandList* cmdList);
    void renderRaster(const D3D* d3d, ID3D12GraphicsCommandList* cmdList) const;
    void compilePtShader(const D3D* d3d);
    void GuiPathTracer(bool resetPT);
    void GuiRaster();
    void guiMain();
    void guiScene();

    Heap m_heap, m_heapRTV;
    CameraController m_camera;

    std::shared_ptr<Shader> m_shader;
    std::shared_ptr<RootSig> m_rootSig, m_rootSigDebug, m_rootSigRaster;

    std::vector<D3D12_INPUT_ELEMENT_DESC> m_shaderILD;
    bool m_shaderDirty = true;

    std::vector<SceneConfig> m_sceneConfigs;
    std::vector<std::shared_ptr<Scene>> m_scenes;
    uint32_t m_currentScene = 0;
    bool m_sceneDirty = true;

    StudioConfig m_studioConfig = {};

    PathTracingContext m_ptContext;
    TextureRTV m_ptOutputTex;

    EnvMap m_envMap;

    RasterContext m_rasterContext;
    std::shared_ptr<Shader> m_shaderRaster;

    XMMATRIX m_projMatrix = {};

#ifdef _DEBUG
    ReadbackManager m_readbackManager;
#endif
};


#endif //PT_SCENESTUDIO_H
