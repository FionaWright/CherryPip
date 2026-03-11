#ifndef PT_SCENESTUDIO_H
#define PT_SCENESTUDIO_H

#include "CBV.h"
#include "PathTracer.h"
#include "System/App.h"
#include "HWI/Heap.h"
#include "HWI/RootSig.h"
#include "HWI/Shader.h"
#include "HWI/Texture.h"
#include "Render/CameraController.h"
#include "Render/DeferredContext.h"
#include "Render/DenoisingManager.h"
#include "Render/EnvMap.h"
#include "Render/PathTracingContext.h"
#include "Render/RasterContext.h"
#include "Render/Skybox.h"
#include "Render/TextureRTV.h"
#include "Render/Transform.h"

#ifdef _DEBUG
#include "Debug/RmseTester.h"
#endif

class BLAS;

struct RasterConfig
{
    RasterDebugMode Mode = eMicrofacetLoWithIndirect;
    bool BloomEnabled = true;
};

enum RenderBackend : uint32_t
{
    eForward,
    eDeferred,
    ePathTracer,
    eSpectralTracer,
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

struct DirLightConfig
{
    bool DirLightDebugLineEnabled = false;
    float DirLightIntensity = 1.0f;
    XMFLOAT3 DirLightDirection = XMFLOAT3(1, -1, 1);
    XMFLOAT3 DirLightColor = XMFLOAT3(1, 1, 1);
};

struct StudioConfig
{
    RenderBackend Backend = ePathTracer;
    RasterConfig Raster = {};
    DenoisingConfig Denoising = {};
    DirLightConfig DirLight = {};

    bool EnvMapEnabled = true;
    bool DebugLinesEnabled = false;
    float EnvMapRotation = 0.0f;
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
    void OnUpdate(D3D* d3d, ID3D12GraphicsCommandList* cmdList, double deltaTime) override;
    void OnPostUpdate(D3D* d3d) override;
    void RenderGUI() override;

    [[nodiscard]] const char* GetName() const override { return "Scene Studio"; }

    float m_AspectRatio = 0;

private:
    void loadAssets(D3D* d3d);
    void loadRasterAssets(const D3D* d3d, ID3D12GraphicsCommandList* cmdList);
    void initScene(D3D* d3d, ID3D12GraphicsCommandList* cmdList, uint32_t configIdx);
    void initCustomScene(D3D* d3d, ID3D12GraphicsCommandList* cmdList);
    bool GBufferPassNeededForDenoiser() const;
    TextureRTV* denoisingPass(const D3D* d3d, ID3D12GraphicsCommandList* cmdList);
    void renderPathTracer(D3D* d3d, ID3D12GraphicsCommandList* cmdList);
    void renderForward(D3D* d3d, ID3D12GraphicsCommandList* cmdList);
    void renderDeferred(D3D* d3d, ID3D12GraphicsCommandList* cmdList);
    void ResetCameraToSceneStart();

    void guiRaster();
    void guiMain();
    void guiScene();
    void guiHeapDebug();

    PathTracer m_pathTracer;

    Heap m_heap, m_heapRTV;
    CameraController m_camera;

    RasterContext m_rasterContext;
    std::shared_ptr<Shader> m_shaderRaster;
    Texture m_texBrdfIntegrationMap;
    DeferredContext m_deferredContext;
    std::shared_ptr<RootSig> m_rootSigRaster;

    std::vector<SceneConfig> m_sceneConfigs;
    std::vector<std::shared_ptr<Scene>> m_scenes;
    uint32_t m_currentScene = 0;

    bool m_shaderDirty = true;
    bool m_sceneDirty = true;
    bool m_pathTracerDirty = false;

    StudioConfig m_studioConfig = {};

    DenoisingManager m_denoisingManager;
    TextureRTV m_rtvPingPong1, m_rtvPingPong2;

    EnvMap m_envMap;
    Skybox m_skybox;
    std::vector<const wchar_t*> m_envMapList;
    uint32_t m_selectedEnvMapIdx = 0;
    bool m_envMapDirty = true;
    bool m_recomputeEnvMapDirLight = false;

#ifdef _DEBUG
    RootSig m_rootSigLine;
    std::shared_ptr<Shader> m_shaderLine;
    DebugLine m_dirLightLine;
    bool m_debugLinesDirty = true;

    RmseTester m_rmseTester;
    uint32_t m_rmseTesterSlot = 0;
#endif

    XMMATRIX m_projMatrix = {};
    TextureRTV* m_finalRTV = &m_rtvPingPong1;

    XMFLOAT2 m_mousePosOnClick = { -1, -1 };
};


#endif //PT_SCENESTUDIO_H
