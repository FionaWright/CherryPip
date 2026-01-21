#include "System/pch.h"
#include "Apps/SceneStudio/Headers/SceneStudio.h"
#include "System/Win32App.h"

#include "ThirdParty/imgui/imgui.h"
#include "Debug/GPUEventScoped.h"
#include "Helper.h"
#include "HWI/D3D.h"
#include "System/Config.h"
#include "System/FileHelper.h"
#include "System/Gui.h"
#include "CBV.h"
#include "MathUtils.h"
#include "HWI/BLAS.h"
#include "HWI/Material.h"
#include "Render/Object.h"
#include "Render/Scene.h"
#include "System/Input.h"
#include "System/ModelLoaderGLTF.h"

#ifdef _DEBUG
#include "Debug/Profiler.h"
#include "Debug/PythonExecutor.h"
#endif

void SceneStudio::OnInit(D3D* d3d)
{
    App::OnInit(d3d);

    m_AspectRatio = static_cast<float>(Config::GetSystem().RtvWidth) / static_cast<float>(Config::GetSystem().
        RtvHeight);

    m_camera.Init(XMFLOAT3(0, 0, 5), 0, PI);

    m_studioConfig.Backend = d3d->GetRayTracingSupported() ? RenderBackend::ePathTracer : RenderBackend::eForward;
    Config::SetUIntFromArg(reinterpret_cast<uint32_t*>(&m_studioConfig.Backend), "--backend");

    loadAssets(d3d);
}

void SceneStudio::OnUpdate(D3D* d3d, ID3D12GraphicsCommandList* cmdList)
{
    if (m_envMapDirty)
    {
        m_envMap.Init(d3d->GetDevice(), cmdList, m_envMapList.at(m_selectedEnvMapIdx), m_studioConfig.EnvMapRotation,
                      &m_heap);
        if (m_studioConfig.Backend != ePathTracer)
        {
            m_envMap.InitCubemap(d3d->GetDevice(), cmdList, &m_heap);
            m_skybox.UpdateCubemap(d3d->GetDevice(), m_envMap.GetCubemap());
            m_skybox.GenerateIrradianceMap(cmdList, &m_heap);
        }
        m_recomputeEnvMapDirLight = true;
        m_envMapDirty = false;
    }

    if (m_sceneDirty)
    {
        if (m_sceneConfigs.at(m_currentScene).SceneIdx == -1)
            initScene(d3d, cmdList, m_currentScene);

        const int sceneIdx = m_sceneConfigs.at(m_currentScene).SceneIdx;

        Scene* currScene = m_scenes.at(sceneIdx).get();
        m_rasterContext.SetScene(currScene);
        m_deferredContext.SetScene(currScene);

        if (d3d->GetRayTracingSupported())
        {
            D12Resource* envMap = m_studioConfig.PT.EnvMapIsEqualArea ? m_envMap.GetEA() : m_envMap.GetPano();
            m_ptContext.BuildScene(d3d->GetDevice(), cmdList, currScene, &m_heap, envMap);
        }

        m_sceneDirty = false;
    }

    if (m_shaderDirty)
    {
        compilePtShader(d3d);
        m_shaderDirty = false;
    }

    m_projMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(Config::GetRender().FoV), m_AspectRatio,
                                            Config::GetRender().NearPlane, Config::GetRender().FarPlane);

    switch (m_studioConfig.Backend)
    {
    case eForward:
        renderForward(d3d, cmdList);
        break;
    case eDeferred:
        renderDeferred(d3d, cmdList);
        break;
    case ePathTracer:
        renderPathTracer(d3d, cmdList);
        break;
    default:
        break;
    }

    const bool moved = m_camera.UpdateCamera();
    if (moved)
        m_ptContext.Reset();
}

void SceneStudio::OnPostUpdate(D3D* d3d)
{
    if (m_recomputeEnvMapDirLight)
    {
        m_studioConfig.DirLightDirection = m_envMap.GetDirectionOfHighestIntensity(d3d, &m_heap);
        m_recomputeEnvMapDirLight = false;
    }
}

void SceneStudio::loadAssets(D3D* d3d)
{
    ID3D12Device* device = d3d->GetDevice();
    const ComPtr<ID3D12GraphicsCommandList> cmdList = d3d->GetAvailableCmdList(D3D12_COMMAND_LIST_TYPE_DIRECT);

    m_heap.Init(device, 10000, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    m_heapRTV.Init(device, 20, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    loadRasterAssets(d3d, cmdList.Get());

    D3D12_STATIC_SAMPLER_DESC samplers[1];
    samplers[0] = {};
    samplers[0].Filter = D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
    samplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    samplers[0].ShaderRegister = 0;

    m_rootSig = std::make_shared<RootSig>();
    m_rootSig->SmartInit(device, 1, 6, 1, true, samplers, _countof(samplers));

#ifdef _DEBUG
    m_rootSigDebug = std::make_shared<RootSig>();
    m_rootSigDebug->SmartInit(device, 2, 6, 1, true, samplers, _countof(samplers));
#endif

    SceneConfig customScene = {
        "Custom"
    };
    m_sceneConfigs.emplace_back(customScene);

    Transform t = {};

    t.SetScale(2.0f);
    SceneConfig sceneCornell = {
        "Cornell Box",
        L"Cornell/scene.gltf",
        t,
        XMFLOAT3(0, 1.5f, 4.5f),
        XMFLOAT2(0, PI),
        true
    };
    m_sceneConfigs.emplace_back(sceneCornell);

    t = {};
    SceneConfig sceneSphere = {
        "Sphere",
        L"Sphere/Sphere.gltf",
        t,
        XMFLOAT3(0, 0, -4.3f),
        XMFLOAT2(0, 0),
        true
    };
    m_sceneConfigs.emplace_back(sceneSphere);

    t = {};
    SceneConfig scenePlane = {
        "FloatPlane",
        L"floatplane.glb",
        t,
        XMFLOAT3(0, 0, -4.3f),
        XMFLOAT2(0, PI),
        true
    };
    m_sceneConfigs.emplace_back(scenePlane);

    t = {};
    t.SetScale(0.3f);
    SceneConfig sceneTeapot = {
        "Utah Teapot",
        L"Utah Teapot/scene.gltf",
        t,
        XMFLOAT3(0, 0, -4.3f),
        XMFLOAT2(0, PI),
        true
    };
    m_sceneConfigs.emplace_back(sceneTeapot);

    t = {};
    t.SetScale(5.0f);
    SceneConfig sceneChess = {
        "Chess",
        L"Chess/Chess.gltf",
        t,
        XMFLOAT3(0, 0.2f, -0.5f),
        XMFLOAT2(0, 0),
        false
    };
    m_sceneConfigs.emplace_back(sceneChess);

    t = {};
    t.SetScale(2.0f);
    SceneConfig sceneLantern = {
        "Lantern",
        L"Lantern/Lantern.gltf",
        t,
        XMFLOAT3(0.967f, 11.963f, 50.213f),
        XMFLOAT2(0, PI),
        true
    };
    m_sceneConfigs.emplace_back(sceneLantern);

    t = {};
    SceneConfig sceneBistro = {
        "Bistro",
        L"GitIgnored/Bistro/bistro.gltf",
        t,
        XMFLOAT3(0.967f, 11.963f, 50.213f),
        XMFLOAT2(0, PI),
        true
    };
    m_sceneConfigs.emplace_back(sceneBistro);

    t = {};
    SceneConfig sceneSponza = {
        "Sponza",
        L"GitIgnored/Sponza/sponza.gltf",
        t,
        XMFLOAT3(1.753f, 1.274f, -0.23f),
        XMFLOAT2(0.105f, 4.747f),
        true
    };
    m_sceneConfigs.emplace_back(sceneSponza);

    t = {};
    SceneConfig sceneMrSpheres = {
        "MetalRough Spheres",
        L"MetalRoughSpheres/MetalRoughSpheres.gltf",
        t,
        XMFLOAT3(-0.5f, -0.25f, 8.5f),
        XMFLOAT2(0, PI),
        true
    };
    m_sceneConfigs.emplace_back(sceneMrSpheres);

    m_envMapList = {
        L"Env Maps/autumn_field_puresky_4k.hdr",
        L"Env Maps/shanghai_bund_4k.hdr",
        L"Env Maps/sunny_rose_garden_4k.hdr",
    };
    m_selectedEnvMapIdx = 0;

    m_currentScene = Config::GetSystem().DefaultSceneIdx;

    auto currScene = m_sceneConfigs.at(m_currentScene);
    if (m_currentScene != 0)
    {
        m_camera.GetCamera().SetPosition(currScene.InitialCamPos);
        m_camera.GetCamera().SetPitchYaw(currScene.InitialCamPitchYaw.x, currScene.InitialCamPitchYaw.y);
    }

    m_ptContext.Init(device, cmdList.Get(), &m_heap);

    m_rtvPingPong1.Init(L"Ping Pong 1", device, &m_heapRTV, Config::GetSystem().RtvWidth, Config::GetSystem().RtvHeight,
                        Config::GetRender().RtvFormat);
    m_rtvPingPong2.Init(L"Ping Pong 2", device, &m_heapRTV, Config::GetSystem().RtvWidth,
                        Config::GetSystem().RtvHeight,
                        Config::GetRender().RtvFormat);

#ifdef _DEBUG
    m_readbackManager.Init(d3d, &m_heap, &m_rtvPingPong1);
#endif

    V(cmdList->Close());
    d3d->ExecuteCommandList(cmdList.Get());
    d3d->Flush();
}

void SceneStudio::loadRasterAssets(const D3D* d3d, ID3D12GraphicsCommandList* cmdList)
{
    D3D12_STATIC_SAMPLER_DESC samplers[1];
    samplers[0] = {};
    samplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    samplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    samplers[0].ShaderRegister = 0;
    samplers[0].MinLOD = 0;
    samplers[0].MaxLOD = D3D12_FLOAT32_MAX;

    m_rootSigRaster = std::make_shared<RootSig>();
    m_rootSigRaster->SmartInit(d3d->GetDevice(), 4, 7, 0, false, samplers, _countof(samplers));

    D3D12_INPUT_ELEMENT_DESC rasterILD[] =
    {
        {
            "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
        },
        {
            "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
        },
        {
            "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
        },
        {
            "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
        },
        {
            "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
        },
    };
    m_shaderRaster = std::make_shared<Shader>();
    m_shaderRaster->InitVsPs(L"Raster/RasterDebugVS.hlsl", L"Raster/RasterDebugPS.hlsl",
                             {rasterILD, _countof(rasterILD)}, d3d->GetDevice(), m_rootSigRaster->Get(), true);

    m_envMap.CreateCubemapResource(d3d->GetDevice());
    m_skybox.Init(d3d->GetDevice(), cmdList, &m_heap, m_envMap.GetCubemap());

    const std::string assetDirectory = wstringToString(FileHelper::GetAssetsPath());
    m_texBrdfIntegrationMap.Init(d3d->GetDevice(), cmdList, assetDirectory + "Textures/BRDF Integration Map.dds");
}

void SceneStudio::initScene(D3D* d3d, ID3D12GraphicsCommandList* cmdList, const uint32_t configIdx)
{
    if (configIdx == 0)
    {
        initCustomScene(d3d, cmdList);
        return;
    }

    CherryPrint("Initializing Scene: " << m_sceneConfigs.at(configIdx).Name << "...");
#ifdef _DEBUG
    Profiler::AddToStack(m_sceneConfigs.at(configIdx).Name.c_str());
#endif

    const SceneConfig config = m_sceneConfigs.at(configIdx);

    GLTFLoadArgs args = {};
    args.Root = m_rootSigRaster;
    args.DefaultShaderIndex = 0;
    args.Shaders = {m_shaderRaster};
    args.ConvertRhToLh = config.ConvertRhToLh;
    args.BrdfIntegrationMap = m_texBrdfIntegrationMap.GetD12Resource();
    args.Skybox = m_envMap.GetCubemap();
    args.IrradianceMap = m_skybox.GetIrradianceMap();

    ModelLoaderGLTF::LoadSplitModel(d3d, cmdList, &m_heap, config.GltfPath, args, config.Transform);

    m_sceneConfigs.at(configIdx).SceneIdx = static_cast<int>(m_scenes.size());
    std::shared_ptr<Scene> scene = std::make_shared<Scene>();
    scene->Init(config.Name.c_str(), config.InitialCamPos, config.InitialCamPitchYaw.x, config.InitialCamPitchYaw.y,
                args.OutObjects);
    m_scenes.emplace_back(scene);

#ifdef _DEBUG
    Profiler::PopAndPrint();
#endif
    CherryPrint("Initialized Scene: " << m_sceneConfigs.at(configIdx).Name);
}

void SceneStudio::initCustomScene(D3D* d3d, ID3D12GraphicsCommandList* cmdList)
{
    CherryPrint("Initializing Custom Scene");
#ifdef _DEBUG
    Profiler::AddToStack(m_sceneConfigs.at(0).Name.c_str());
#endif

    GLTFLoadArgs args;
    args.Root = m_rootSigRaster;
    args.DefaultShaderIndex = 0;
    args.Shaders = {m_shaderRaster};

    Transform t = {};

    // Empty cornell box
    {
        t.SetScale(2.0f);
        args.CullingWhiteList = {
            "Object_0", "Object_1", "Object_2", "Object_3", "Object_4", "Object_5", "Object_6", "Object_7", "Object_8"
        };
        ModelLoaderGLTF::LoadSplitModel(d3d, cmdList, &m_heap, L"Cornell/scene.gltf", args, t);
        args.OutObjects.back()->GetTransform()->SetPosition(0.25f, 0.02f, -0.5f);
        args.OutObjects.back()->GetTransform()->SetRotationE(-1.57f, 0.5f, 0.0f);
        args.OutObjects.back()->GetTransform()->SetScale(2.5f);
        args.OutObjects.back()->GetMaterial()->GetData()->DiffuseProbability = 0.0f;
        args.CullingWhiteList.clear();
        t = {};
    }

    // Sphere
    if (false)
    {
        t.SetPosition(0, 1, 0);
        ModelLoaderGLTF::LoadSplitModel(d3d, cmdList, &m_heap, L"Sphere/Sphere.gltf", args, t);
        args.OutObjects.back()->GetMaterial()->GetData()->BindlessTexDiffuse = 0;
        args.OutObjects.back()->GetMaterial()->GetData()->Metalness = 0;
        args.OutObjects.back()->GetMaterial()->GetData()->Roughness = 0;
        args.OutObjects.back()->GetMaterial()->GetData()->Flags = PtMaterialFlags::eIsGlass;
        t = {};
    }

    constexpr auto initialCamPos = XMFLOAT3(0, 1.5f, 4.5f);
    constexpr auto initialCamRot = XMFLOAT2(0, PI);
    m_camera.GetCamera().SetPosition(initialCamPos);
    m_camera.GetCamera().SetPitchYaw(initialCamRot.x, initialCamRot.y);

    m_sceneConfigs.at(0).SceneIdx = static_cast<int>(m_scenes.size());
    std::shared_ptr<Scene> scene = std::make_shared<Scene>();
    scene->Init("Custom", initialCamPos, initialCamRot.x, initialCamRot.y, args.OutObjects);
    m_scenes.emplace_back(scene);

#ifdef _DEBUG
    Profiler::PopAndPrint();
#endif
    CherryPrint("Initialized Custom Scene");
}

void copyRtvTex(ID3D12GraphicsCommandList* cmdList, D12Resource* d3dRTV, D12Resource* rtvTex)
{
    rtvTex->Transition(cmdList, D3D12_RESOURCE_STATE_COPY_SOURCE);
    d3dRTV->Transition(cmdList, D3D12_RESOURCE_STATE_COPY_DEST);
    ID3D12Resource* srcResource = rtvTex->GetResource();
    d3dRTV->CopyTextureInto(cmdList, srcResource, Config::GetSystem().WindowAppGuiWidth);
    rtvTex->Transition(cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);
}

void SceneStudio::denoisingPass(D3D* d3d, ID3D12GraphicsCommandList* cmdList)
{
    // GBuffer Pass
    if (m_studioConfig.Denoising.Type == eATrous)
    {
        if (!m_deferredContext.IsInitialized())
            m_deferredContext.Init(d3d->GetDevice(), cmdList, &m_heapRTV, &m_heap);

        const Skybox* skybox = m_studioConfig.EnvMapEnabled ? &m_skybox : nullptr;
        m_deferredContext.RenderGBuffer(d3d, cmdList, m_camera.GetViewMatrix(), m_projMatrix, skybox);
    }

    if (!m_denoisingManager.IsInitialized())
        m_denoisingManager.Init(d3d->GetDevice(), cmdList, &m_heap, m_rtvPingPong1.GetD12Resource(),
                                m_rtvPingPong2.GetD12Resource(),
                                m_deferredContext.GetNormalsDepth());

    TextureRTV* outputRTV = nullptr;
    switch (m_studioConfig.Denoising.Type)
    {
    case eBox:
        outputRTV = m_denoisingManager.DenoiseBox(cmdList, &m_rtvPingPong1, &m_rtvPingPong2,
                                                  m_studioConfig.Denoising.BoxRadius);
        break;
    case eGaussian:
        outputRTV = m_denoisingManager.DenoiseGauss(cmdList, &m_rtvPingPong1, &m_rtvPingPong2,
                                                    m_studioConfig.Denoising.BoxRadius);
        break;
    case eMedian:
        outputRTV = m_denoisingManager.DenoiseMedian(cmdList, &m_rtvPingPong1, &m_rtvPingPong2);
        break;
    case eATrous:
        outputRTV = m_denoisingManager.DenoiseATrous(cmdList, m_camera.GetViewMatrix(), m_projMatrix,
                                                     &m_rtvPingPong1, &m_rtvPingPong2,
                                                     m_studioConfig.Denoising.ATrousIterations,
                                                     m_studioConfig.Denoising.ATrousPhiC,
                                                     m_studioConfig.Denoising.ATrousPhiN,
                                                     m_studioConfig.Denoising.ATrousPhiP);
        break;
    default:
        throw std::exception("Unsupported Denoiser");
    }

    copyRtvTex(cmdList, d3d->GetRtv(), outputRTV->GetD12Resource());
}

void SceneStudio::renderPathTracer(D3D* d3d, ID3D12GraphicsCommandList* cmdList)
{
    if (!d3d->GetRayTracingSupported())
    {
        std::cout << "ERROR: Ray-Tracing not supported!!!" << std::endl;
        return;
    }

    const float fRtvWidth = static_cast<float>(Config::GetSystem().RtvWidth);
    const float fRtvHeight = static_cast<float>(Config::GetSystem().RtvHeight);

    const CD3DX12_VIEWPORT viewport(0.0f, 0.0f, fRtvWidth, fRtvHeight);
    const CD3DX12_RECT scissorRect(0, 0, Config::GetSystem().RtvWidth, Config::GetSystem().RtvHeight);

    cmdList->RSSetViewports(1, &viewport);
    cmdList->RSSetScissorRects(1, &scissorRect);

    m_heap.SetHeap(cmdList);

    // Main Pass (Into PP1)
    {
        m_rtvPingPong1.GetD12Resource()->Transition(cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);
        const UINT rtvIdx = m_rtvPingPong1.GetHeapIdx();
        const auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_heapRTV.GetCPUHandle(), rtvIdx,
                                                          m_heapRTV.GetIncrementSize());
        cmdList->OMSetRenderTargets(1, &handle, FALSE, nullptr);

        ID3D12RootSignature* rootSig = m_studioConfig.PT.DebugMode ? m_rootSigDebug->Get() : m_rootSig->Get();
        const int debugBufferIdx = m_studioConfig.PT.DebugMode
                                       ? static_cast<uint32_t>(m_studioConfig.PT.DebugBufferIdx)
                                       : -1;

        m_ptContext.Render(cmdList, rootSig, m_shader->GetPSO(), &m_camera.GetCamera(), &m_heap, m_projMatrix,
                           m_studioConfig.PT, m_studioConfig.DirLightIntensity, m_studioConfig.DirLightColor,
                           m_studioConfig.DirLightDirection, debugBufferIdx);
    }

#ifdef _DEBUG
    // Readback Pass (PP1 to RTV)
    if (m_studioConfig.PT.ReadbackEnabled)
    {
        m_readbackManager.ReadbackPass(d3d, cmdList, &m_rtvPingPong1, m_studioConfig.PT.ReadbackEveryFrame);
        copyRtvTex(cmdList, d3d->GetRtv(), m_rtvPingPong1.GetD12Resource());
        return;
    }
#endif

    if (m_studioConfig.Denoising.Enabled)
    {
        denoisingPass(d3d, cmdList);
        return;
    }

    // PP1 to RTV
    copyRtvTex(cmdList, d3d->GetRtv(), m_rtvPingPong1.GetD12Resource());
}

void SceneStudio::renderForward(D3D* d3d, ID3D12GraphicsCommandList* cmdList)
{
    m_heap.SetHeap(cmdList);

    // Update CBVs
    {
        CbvRasterVS cbvRasterVs{};
        cbvRasterVs.CameraPos = m_camera.GetCamera().GetPosition();

        CbvRasterDebug cbvRasterDebug{};
        cbvRasterDebug.Mode = m_studioConfig.Raster.Mode;

        CbvForwardLighting cbvForwardLighting{};
        cbvForwardLighting.DirLightDir = m_studioConfig.DirLightDirection;
        cbvForwardLighting.MaxCubemapMipMaps = m_envMap.GetCubemap()->GetDesc().MipLevels;

        const int sceneIdx = m_sceneConfigs.at(m_currentScene).SceneIdx;

        const auto& currScene = m_scenes.at(sceneIdx);
        auto& objects = currScene->GetObjects();
        for (int i = 0; i < objects.size(); ++i)
        {
            objects[i]->GetMaterial()->UpdateCBV(1, &cbvRasterVs);
            objects[i]->GetMaterial()->UpdateCBV(2, &cbvForwardLighting);
            objects[i]->GetMaterial()->UpdateCBV(3, &cbvRasterDebug);
        }
    }

    // Main Pass (Into PP1)
    {
        m_rtvPingPong1.GetD12Resource()->Transition(cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);
        const UINT rtvIdx = m_rtvPingPong1.GetHeapIdx();
        const auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_heapRTV.GetCPUHandle(), rtvIdx,
                                                          m_heapRTV.GetIncrementSize());

        const Skybox* skybox = m_studioConfig.EnvMapEnabled ? &m_skybox : nullptr;
        m_rasterContext.Render(d3d, cmdList, m_camera.GetViewMatrix(), m_projMatrix, handle, skybox);
    }

    if (m_studioConfig.Denoising.Enabled)
    {
        denoisingPass(d3d, cmdList);
        return;
    }

    copyRtvTex(cmdList, d3d->GetRtv(), m_rtvPingPong1.GetD12Resource());
}

void SceneStudio::renderDeferred(D3D* d3d, ID3D12GraphicsCommandList* cmdList)
{
    m_heap.SetHeap(cmdList);

    // GBuffer Pass
    {
        if (!m_deferredContext.IsInitialized())
            m_deferredContext.Init(d3d->GetDevice(), cmdList, &m_heapRTV, &m_heap);

        const Skybox* skybox = m_studioConfig.EnvMapEnabled ? &m_skybox : nullptr;
        m_deferredContext.RenderGBuffer(d3d, cmdList, m_camera.GetViewMatrix(), m_projMatrix, skybox);
    }

    // Lighting Pass
    {
        m_deferredContext.RenderLighting(d3d, cmdList, &m_heap, m_projMatrix, &m_rtvPingPong1,
                                         m_studioConfig.DirLightDirection, m_envMap.GetCubemap(),
                                         m_skybox.GetIrradianceMap(), m_texBrdfIntegrationMap.GetD12Resource(),
                                         m_studioConfig.Raster.Mode);
    }

    copyRtvTex(cmdList, d3d->GetRtv(), m_rtvPingPong1.GetD12Resource());
}

void SceneStudio::compilePtShader(const D3D* d3d)
{
    if (!d3d->GetRayTracingSupported())
        return;

    const WCHAR* shaderMap[] = {
        L"Path-Tracing/Entry/LambDiffPS.hlsl",
        L"Path-Tracing/Entry/GlossyPS.hlsl",
        L"Path-Tracing/Entry/GlassPS.hlsl",
        L"Path-Tracing/Entry/GgxSmithMicrofacetPS.hlsl",
        L"Path-Tracing/Entry/FurnaceHdReflectPS.hlsl",
        L"Path-Tracing/Entry/FurnaceHhEmitPS.hlsl"
    };
    const WCHAR* shaderPath = shaderMap[m_studioConfig.PT.Mode];

    CherryPrint("Loading Shader: " << wstringToString(shaderPath));

    ID3D12RootSignature* rootSig = m_rootSig->Get();
    std::vector<const WCHAR*> args = {};
    if (m_studioConfig.PT.DebugMode)
    {
        args.push_back(L"-DDEBUG_BUFFER");
        rootSig = m_rootSigDebug->Get();
    }
    if (m_studioConfig.EnvMapEnabled)
        args.push_back(L"-DENV_MAP_ENABLED");
    if (m_studioConfig.PT.EnvMapIsEqualArea)
        args.push_back(L"-DENV_MAP_EA");
    if (m_studioConfig.PT.DirLightEnabled)
        args.push_back(L"-DDIR_LIGHT_ENABLED");
    if (m_studioConfig.PT.NormalMapsEnabled)
        args.push_back(L"-DNORMAL_MAPS_ENABLED");
    if (m_studioConfig.PT.RussianRouletteEnabled)
        args.push_back(L"-DRUSSIAN_ROULETTE_ENABLED");

    m_shaderILD =
    {
        {
            "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
        },
        {
            "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
        }
    };
    const D3D12_INPUT_LAYOUT_DESC ild = {m_shaderILD.data(), static_cast<UINT>(m_shaderILD.size())};

    m_shader = std::make_shared<Shader>();
    m_shader->InitVsPs(L"FullScreenTriangleVS.hlsl", shaderPath, ild, d3d->GetDevice(), rootSig, false, args);
}
