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
#include "Render/DeferredContext.h"
#include "Render/DenoisingManager.h"
#include "Render/EnvMap.h"
#include "Render/Object.h"
#include "Render/PathTracingContext.h"
#include "Render/RasterContext.h"
#include "Render/Skybox.h"
#include "Render/TextureRTV.h"
#include "Render/Transform.h"

#ifdef _DEBUG
#include "ReadbackManager.h"
#endif

class BLAS;
using Microsoft::WRL::ComPtr;

struct RasterConfig
{
    RasterDebugMode Mode = eDirLightingTex;
};

enum RenderBackend : uint32_t
{
    eForward,
    ePathTracer,
    MAX_COUNT,
};

struct DenoisingConfig
{
    bool Enabled = false;
    int BoxRadius = 1;
    int ATrousIterations = 5;
    DenoisingType Type = eATrous;
    float ATrousPhiC = 1.0f; // Tune this third
    float ATrousPhiN = 0.1f; // Tune this first
    float ATrousPhiP = 1.0f; // Tune this second
};

struct StudioConfig
{
    RenderBackend Backend = ePathTracer;
    PtConfig PT = {};
    RasterConfig Raster = {};
    DenoisingConfig Denoising = {};

    bool EnvMapEnabled = true;
    float EnvMapRotation = 0.0f;
    float DirLightIntensity = 100.0f;
    XMFLOAT3 DirLightDirection = XMFLOAT3(1, -1, 1);
    XMFLOAT3 DirLightColor = XMFLOAT3(1, 1, 1);
};

struct SceneConfig
{
    std::string Name;
    std::wstring GltfPath;
    Transform Transform;
    XMFLOAT3 InitialCamPos;
    XMFLOAT2 InitialCamPitchYaw;
    bool ConvertRhToLh = true;

    int SceneIdx = -1;
};

class SceneStudio final : public App
{
public:
    void OnInit(D3D* d3d) override;
    void OnUpdate(D3D* d3d, ID3D12GraphicsCommandList* cmdList) override;
    void OnPostUpdate(D3D* d3d) override;
    void RenderGUI() override;

    [[nodiscard]] const char* GetName() const override { return "Scene Studio"; }

    float m_AspectRatio = 0;

private:
    void loadAssets(D3D* d3d);
    void loadRasterAssets(const D3D* d3d);
    void initScene(D3D* d3d, ID3D12GraphicsCommandList* cmdList, uint32_t configIdx);
    void initCustomScene(D3D* d3d, ID3D12GraphicsCommandList* cmdList);
    void renderPathTracer(D3D* d3d, ID3D12GraphicsCommandList* cmdList);
    void renderRaster(D3D* d3d, ID3D12GraphicsCommandList* cmdList);
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
    DenoisingManager m_denoisingManager;
    TextureRTV m_rtvPingPong1, m_rtvPingPong2;

    DeferredContext m_deferredContext;

    EnvMap m_envMap;
    Skybox m_skybox;
    std::vector<const wchar_t*> m_envMapList;
    uint32_t m_selectedEnvMapIdx = 0;
    bool m_envMapDirty = true;
    bool m_recomputeEnvMapDirLight = false;

    RasterContext m_rasterContext;
    std::shared_ptr<Shader> m_shaderRaster;

    XMMATRIX m_projMatrix = {};

#ifdef _DEBUG
    ReadbackManager m_readbackManager;
#endif
};


#endif //PT_SCENESTUDIO_H
