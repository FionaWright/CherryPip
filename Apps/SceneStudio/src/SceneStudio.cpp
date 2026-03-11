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
    Config::SetUIntFromArg(reinterpret_cast<uint32_t*>(&m_studioConfig.PT.LightingModel), "--lightingModel");
    Config::SetUIntFromArg(&m_studioConfig.PT.SPP, "--spp");
    Config::SetBoolFromArg(&m_studioConfig.EnvMapEnabled, "--envMapEnabled");
    Config::SetBoolFromArg(&m_studioConfig.PT.DirLightEnabled, "--dirLight");
    Config::SetBoolFromArg(&m_studioConfig.PT.RussianRouletteEnabled, "--russianRoulette");

    loadAssets(d3d);
}

void SceneStudio::OnUpdate(D3D* d3d, ID3D12GraphicsCommandList* cmdList, double deltaTime)
{
    if (m_envMapDirty)
    {
        m_envMap.Init(d3d->GetDevice(), cmdList, m_envMapList.at(m_selectedEnvMapIdx), m_studioConfig.EnvMapRotation,
                      &m_heap);
        if (m_studioConfig.Backend == eForward || m_studioConfig.Backend == eDeferred || GBufferPassNeededForDenoiser())
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

        m_pathTracer.BuildScene(d3d, cmdList, &m_envMap, currScene, &m_heap);

        m_sceneDirty = false;
    }

#ifdef _DEBUG
    if (Input::IsMouseLeftDown())
    {
        const XMFLOAT2 mousePos = Input::GetMousePos();
        const uint32_t minX = Config::GetSystem().WindowAppGuiWidth;
        const uint32_t maxX = Config::GetSystem().WindowAppGuiWidth + Config::GetSystem().RtvWidth;
        if (mousePos.x >= minX && mousePos.x < maxX)
            m_mousePosOnClick = {mousePos.x - minX, mousePos.y};
    }
#endif

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
    case eSpectralTracer:
        m_pathTracer.Update(d3d, &m_heap, m_shaderDirty, m_studioConfig.Backend == eSpectralTracer);
        m_shaderDirty = false;

        m_pathTracer.Render(d3d, cmdList);
        break;
    default:
        break;
    }

    const bool moved = m_camera.UpdateCamera(deltaTime);
    if (moved)
        m_pathTracer.Reset();
}

void SceneStudio::OnPostUpdate(D3D* d3d)
{
    if (m_recomputeEnvMapDirLight)
    {
        m_studioConfig.DirLight.DirLightDirection = m_envMap.GetDirectionOfHighestIntensity(d3d, &m_heap);
        m_recomputeEnvMapDirLight = false;
        m_debugLinesDirty = true;
    }

    if (Input::IsKeyDown(KeyCode::R))
    {
        m_pathTracer.Reset();
        ResetCameraToSceneStart();
    }

#ifdef _DEBUG
    if (m_studioConfig.DebugLinesEnabled && m_debugLinesDirty)
    {
        const XMFLOAT3 start = Mult(m_studioConfig.DirLight.DirLightDirection, 1000.0f);
        const XMFLOAT3 end = Mult(m_studioConfig.DirLight.DirLightDirection, -1000.0f);
        constexpr XMFLOAT3 color = XMFLOAT3(1, 1, 0);
        m_dirLightLine.Update(d3d, &start, &end, &color);
        m_debugLinesDirty = false;
    }
#endif

    m_pathTracer.PostUpdate(d3d, &m_heap, m_finalRTV);
}

void SceneStudio::loadAssets(D3D* d3d)
{
    ID3D12Device* device = d3d->GetDevice();
    const ComPtr<ID3D12GraphicsCommandList> cmdList = d3d->GetAvailableCmdList(D3D12_COMMAND_LIST_TYPE_DIRECT);

    // TODO: Only set high heap if Bistro initial scene, don't allow changing scene after?
    m_heap.Init("SRV/CBV/UAV Heap", device, 70000, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    m_heapRTV.Init("RTV Heap", device, 20, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    loadRasterAssets(d3d, cmdList.Get());

    D3D12_STATIC_SAMPLER_DESC samplers[1];
    samplers[0] = {};
    samplers[0].Filter = D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
    samplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    samplers[0].ShaderRegister = 0;

    SceneConfig customScene = {
        "(" + std::to_string(m_sceneConfigs.size()) +") " + "Custom"
    };
    m_sceneConfigs.emplace_back(customScene);

    Transform t = {};

    t.SetScale(2.0f);
    SceneConfig sceneCornell = {
        "(" + std::to_string(m_sceneConfigs.size()) +") " + "Cornell Box",
        L"Cornell/scene.gltf",
        t,
        XMFLOAT3(0, 1.5f, 4.5f),
        XMFLOAT2(0, PI),
        true
    };
    m_sceneConfigs.emplace_back(sceneCornell);

    t = {};
    SceneConfig sceneSphere = {
        "(" + std::to_string(m_sceneConfigs.size()) +") " + "Sphere",
        L"Sphere/Sphere.gltf",
        t,
        XMFLOAT3(0, 0, -4.3f),
        XMFLOAT2(0, 0),
        true
    };
    m_sceneConfigs.emplace_back(sceneSphere);

    t = {};
    SceneConfig scenePlane = {
        "(" + std::to_string(m_sceneConfigs.size()) +") " + "FloatPlane",
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
        "(" + std::to_string(m_sceneConfigs.size()) +") " + "Utah Teapot",
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
        "(" + std::to_string(m_sceneConfigs.size()) +") " + "Chess",
        L"Chess/Chess.gltf",
        t,
        XMFLOAT3(0.45f, 0.919f, -1.434f),
        XMFLOAT2(0.66f, -0.36f),
        false
    };
    m_sceneConfigs.emplace_back(sceneChess);

    t = {};
    t.SetScale(2.0f);
    SceneConfig sceneLantern = {
        "(" + std::to_string(m_sceneConfigs.size()) +") " + "Lantern",
        L"Lantern/Lantern.gltf",
        t,
        XMFLOAT3(0.967f, 11.963f, 50.213f),
        XMFLOAT2(0, PI),
        true
    };
    m_sceneConfigs.emplace_back(sceneLantern);

    t = {};
    SceneConfig sceneBistro = {
        "(" + std::to_string(m_sceneConfigs.size()) +") " + "Bistro",
        L"GitIgnored/Bistro/bistro.gltf",
        t,
        XMFLOAT3(0.967f, 11.963f, 50.213f),
        XMFLOAT2(0, PI),
        false
    };
    m_sceneConfigs.emplace_back(sceneBistro);

    t = {};
    SceneConfig sceneSponza = {
        "(" + std::to_string(m_sceneConfigs.size()) +") " + "Sponza",
        L"GitIgnored/Sponza/sponza.gltf",
        t,
        XMFLOAT3(1.753f, 1.274f, -0.23f),
        XMFLOAT2(0.105f, 4.747f),
        false
    };
    m_sceneConfigs.emplace_back(sceneSponza);

    t = {};
    SceneConfig sceneMrSpheres = {
        "(" + std::to_string(m_sceneConfigs.size()) +") " + "MetalRough Spheres",
        L"MetalRoughSpheres/MetalRoughSpheres.gltf",
        t,
        XMFLOAT3(-0.5f, -0.25f, 8.5f),
        XMFLOAT2(0, PI),
        true
    };
    m_sceneConfigs.emplace_back(sceneMrSpheres);

    t = {};
    SceneConfig sceneAnisoDiscs = {
        "(" + std::to_string(m_sceneConfigs.size()) +") " + "Aniso Discs",
        L"AnisoDiscs/AnisotropyDiscTest.gltf",
        t,
        XMFLOAT3(0.17f, 1.471f, 3.542f),
        XMFLOAT2(0.0f, PI),
        true
    };
    m_sceneConfigs.emplace_back(sceneAnisoDiscs);

    t = {};
    t.SetScale(6.0f);
    SceneConfig sceneBarnLamp = {
        "(" + std::to_string(m_sceneConfigs.size()) +") " + "Barn Lamp",
        L"BarnLamp/AnisotropyBarnLamp.gltf",
        t,
        XMFLOAT3(0.366f, -0.104f, 0.552f),
        XMFLOAT2(0.135f, 4.012f),
        true
    };
    m_sceneConfigs.emplace_back(sceneBarnLamp);

    t = {};
    SceneConfig sceneWhiteLands = {
        "(" + std::to_string(m_sceneConfigs.size()) +") " + "White Lands",
        L"GitIgnored/WhiteLands/WhiteLands.gltf",
        t,
        XMFLOAT3(0,0,0),
        XMFLOAT2(0, PI),
        false
    };
    m_sceneConfigs.emplace_back(sceneWhiteLands);

    t = {};
    t.SetScale(3.0f);
    SceneConfig sceneCozyKitchen = {
        "(" + std::to_string(m_sceneConfigs.size()) +") " + "CozyKitchen",
        L"GitIgnored/CozyKitchen/CozyKitchen.gltf",
        t,
        XMFLOAT3(1.339f, 1.728f, 0.989f),
        XMFLOAT2(0.12f, 3.997f),
        false
    };
    m_sceneConfigs.emplace_back(sceneCozyKitchen);

    t = {};
    t.SetScale(2.0f);
    SceneConfig sceneAutumn = {
        "(" + std::to_string(m_sceneConfigs.size()) +") " + "Autumn",
        L"GitIgnored/Autumn/Autumn.gltf",
        t,
        XMFLOAT3(1.339f, 1.728f, 0.989f),
        XMFLOAT2(0.12f, 3.997f),
        false
    };
    m_sceneConfigs.emplace_back(sceneAutumn);

    t = {};
    t.SetScale(2.0f);
    SceneConfig sceneJunkshop = {
        "(" + std::to_string(m_sceneConfigs.size()) +") " + "Junkshop",
        L"GitIgnored/Junkshop/Junkshop.gltf",
        t,
        XMFLOAT3(1.339f, 1.728f, 0.989f),
        XMFLOAT2(0.12f, 3.997f),
        false
    };
    m_sceneConfigs.emplace_back(sceneJunkshop);

    t = {};
    t.SetScale(2.0f);
    SceneConfig sceneLoneMonk = {
        "(" + std::to_string(m_sceneConfigs.size()) +") " + "LoneMonk",
        L"GitIgnored/LoneMonk/LoneMonk.gltf",
        t,
        XMFLOAT3(1.339f, 1.728f, 0.989f),
        XMFLOAT2(0.12f, 3.997f),
        false
    };
    m_sceneConfigs.emplace_back(sceneLoneMonk);

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
        ResetCameraToSceneStart();
    }

    m_rtvPingPong1.Init(L"Ping Pong 1", device, &m_heapRTV, Config::GetSystem().RtvWidth, Config::GetSystem().RtvHeight,
                        Config::GetRender().RtvFormat);
    m_rtvPingPong2.Init(L"Ping Pong 2", device, &m_heapRTV, Config::GetSystem().RtvWidth,
                        Config::GetSystem().RtvHeight,
                        Config::GetRender().RtvFormat);

#ifdef _DEBUG
    // Initialize Debug Lines
    {
        m_rootSigLine.SmartInit(d3d->GetDevice(), 1, 0);

        const std::vector<D3D12_INPUT_ELEMENT_DESC> ildDesc =
        {
            {
                "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
                D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
            },
            {
                "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
                D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
            }
        };
        const D3D12_INPUT_LAYOUT_DESC ild = { ildDesc.data(), static_cast<uint32_t>(ildDesc.size())};
        m_shaderLine = std::make_shared<Shader>();
        m_shaderLine->InitVsPs(L"LineVSPS.hlsl", L"LineVSPS.hlsl", ild, device, m_rootSigLine.Get(), true, {}, 1, D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE);

        m_dirLightLine.Init(d3d, &m_heap, m_shaderLine);
    }
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
    m_rootSigRaster->SmartInit(d3d->GetDevice(), 5, 3, 0, true, samplers, _countof(samplers));

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
        //{
        //    "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
        //    D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
        //},
        //{
        //    "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
        //    D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
        //},
    };
    m_shaderRaster = std::make_shared<Shader>();
    m_shaderRaster->InitVsPs(L"Raster/ForwardVS.hlsl", L"Raster/ForwardPS.hlsl",
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
    m_heap.PrintHeapInfo();
    m_heapRTV.PrintHeapInfo();
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
    args.BrdfIntegrationMap = m_texBrdfIntegrationMap.GetD12Resource();
    args.Skybox = m_envMap.GetCubemap();
    args.IrradianceMap = m_skybox.GetIrradianceMap();

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
        args.OutObjects.back()->GetMaterialForward()->GetData()->DiffuseProbability = 0.0f;
        args.CullingWhiteList.clear();
        t = {};
    }

    // Sphere
    if (false)
    {
        t.SetPosition(0, 1, 0);
        ModelLoaderGLTF::LoadSplitModel(d3d, cmdList, &m_heap, L"Sphere/Sphere.gltf", args, t);
        args.OutObjects.back()->GetMaterialForward()->GetData()->BindlessTexDiffuse = 0;
        args.OutObjects.back()->GetMaterialForward()->GetData()->Metalness = 0;
        args.OutObjects.back()->GetMaterialForward()->GetData()->Roughness = 0;
        args.OutObjects.back()->GetMaterialForward()->GetData()->Flags = PtMaterialFlags::eIsGlass;
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

bool SceneStudio::GBufferPassNeededForDenoiser() const
{
    return m_studioConfig.Denoising.Type == eATrous;
}

TextureRTV* SceneStudio::denoisingPass(const D3D* d3d, ID3D12GraphicsCommandList* cmdList)
{
    // GBuffer Pass
    if (GBufferPassNeededForDenoiser())
    {
        if (!m_deferredContext.IsInitialized())
            m_deferredContext.Init(d3d->GetDevice(), cmdList, &m_heapRTV, &m_heap);

        const Skybox* skybox = m_studioConfig.EnvMapEnabled ? &m_skybox : nullptr;
        m_deferredContext.RenderGBuffer(d3d, cmdList, m_camera.GetViewMatrix(), m_projMatrix, &m_heap, skybox);
    }

    if (!m_denoisingManager.IsInitialized())
        m_denoisingManager.Init(d3d->GetDevice(), cmdList, &m_heap);

    TextureRTV* outputRTV = nullptr;
    switch (m_studioConfig.Denoising.Type)
    {
    case eBox:
        outputRTV = m_denoisingManager.DenoiseBox(d3d->GetDevice(), cmdList, &m_heap,  &m_rtvPingPong1, &m_rtvPingPong2,
                                                  m_studioConfig.Denoising.BoxRadius);
        break;
    case eGaussian:
        outputRTV = m_denoisingManager.DenoiseGauss(d3d->GetDevice(), cmdList, &m_heap, &m_rtvPingPong1, &m_rtvPingPong2,
                                                    m_studioConfig.Denoising.BoxRadius);
        break;
    case eMedian:
        outputRTV = m_denoisingManager.DenoiseMedian(d3d->GetDevice(), cmdList, &m_heap, &m_rtvPingPong1, &m_rtvPingPong2);
        break;
    case eATrous:
        outputRTV = m_denoisingManager.DenoiseATrous(d3d->GetDevice(), cmdList, &m_heap,  m_camera.GetViewMatrix(), m_projMatrix,
                                                     &m_rtvPingPong1, &m_rtvPingPong2, m_deferredContext.GetNormalsDepth(),
                                                     m_studioConfig.Denoising.ATrousIterations,
                                                     m_studioConfig.Denoising.ATrousPhiC,
                                                     m_studioConfig.Denoising.ATrousPhiN,
                                                     m_studioConfig.Denoising.ATrousPhiP);
        break;
    default:
        throw std::exception("Unsupported Denoiser");
    }

    return outputRTV;
}

void SceneStudio::renderPathTracer(D3D* d3d, ID3D12GraphicsCommandList* cmdList)
{
    m_heap.SetHeap(cmdList);
    
    m_rtvPingPong1.GetD12Resource()->Transition(cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);
    const UINT rtvIdx = m_rtvPingPong1.GetHeapIdx();
    const auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_heapRTV.GetCPUHandle(), rtvIdx,
                                                        m_heapRTV.GetIncrementSize());

    m_pathTracer.Render(d3d, cmdList, &m_camera.GetCamera(), m_projMatrix, handle);

    m_finalRTV = &m_rtvPingPong1;

#ifdef _DEBUG
    if (m_studioConfig.DebugLinesEnabled)
    {
        GPU_SCOPE(cmdList, "Debug Lines");

        // TODO: Not optimal, take depth from path-tracer probably. Not high priority as its debug
        // GBuffer Pass for Depth Buffer
        {
            if (!m_deferredContext.IsInitialized())
                m_deferredContext.Init(d3d->GetDevice(), cmdList, &m_heapRTV, &m_heap);
            m_deferredContext.RenderGBuffer(d3d, cmdList, m_camera.GetViewMatrix(), m_projMatrix, &m_heap, nullptr);
        }

        m_finalRTV->GetD12Resource()->Transition(cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);
        const UINT rtvIdx = m_finalRTV->GetHeapIdx();
        const auto rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_heapRTV.GetCPUHandle(), rtvIdx, m_heapRTV.GetIncrementSize());
        const CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(d3d->GetDsvHeapStart(), d3d->GetFrameIndex(), d3d->GetDsvDescriptorSize());
        cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

        cmdList->SetGraphicsRootSignature(m_rootSigLine.Get());
        const XMMATRIX vp = m_camera.GetViewMatrix() * m_projMatrix;

        if (m_studioConfig.DirLight.DirLightDebugLineEnabled)
            m_dirLightLine.Render(cmdList, vp);

        m_pathTracer.RenderLines(d3d, cmdList, vp);
    }
#endif

    if (m_studioConfig.Denoising.Enabled)
    {
        m_finalRTV = denoisingPass(d3d, cmdList);
        copyRtvTex(cmdList, d3d->GetRtv(), m_finalRTV->GetD12Resource());
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
        cbvForwardLighting.DirLightDir = m_studioConfig.DirLight.DirLightDirection;
        cbvForwardLighting.MaxCubemapMipMaps = m_envMap.GetCubemap()->GetDesc().MipLevels;

        const int sceneIdx = m_sceneConfigs.at(m_currentScene).SceneIdx;

        const auto& currScene = m_scenes.at(sceneIdx);
        auto& objects = currScene->GetObjects();
        for (int i = 0; i < objects.size(); ++i)
        {
            objects[i]->GetMaterialForward()->UpdateCBV(1, &cbvRasterVs);
            objects[i]->GetMaterialForward()->UpdateCBV(2, &cbvForwardLighting);
            objects[i]->GetMaterialForward()->UpdateCBV(3, &cbvRasterDebug);
        }
    }

    // Main Pass (Into PP1)
    {
        m_rtvPingPong1.GetD12Resource()->Transition(cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);
        const UINT rtvIdx = m_rtvPingPong1.GetHeapIdx();
        const auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_heapRTV.GetCPUHandle(), rtvIdx,
                                                          m_heapRTV.GetIncrementSize());

        cmdList->SetGraphicsRootSignature(m_rootSigRaster->Get());

        const Skybox* skybox = m_studioConfig.EnvMapEnabled ? &m_skybox : nullptr;
        m_rasterContext.Render(d3d, cmdList, m_camera.GetViewMatrix(), m_projMatrix, handle, &m_heap, skybox);
    }

#ifdef _DEBUG
    // Debug Line Pass
    if (m_studioConfig.DebugLinesEnabled)
    {
        GPU_SCOPE(cmdList, "Debug Lines");

        cmdList->SetGraphicsRootSignature(m_rootSigLine.Get());
        const XMMATRIX vp = m_camera.GetViewMatrix() * m_projMatrix;

        if (m_studioConfig.DirLight.DirLightDebugLineEnabled)
            m_dirLightLine.Render(cmdList, vp);

        for (int i = 0; i < m_pathVisualizationLines.size(); i++)
            m_pathVisualizationLines[i]->Render(cmdList, vp);
    }
#endif

    m_finalRTV = &m_rtvPingPong1;

    if (m_studioConfig.Denoising.Enabled)
        m_finalRTV = denoisingPass(d3d, cmdList);

    copyRtvTex(cmdList, d3d->GetRtv(), m_finalRTV->GetD12Resource());
}

void SceneStudio::renderDeferred(D3D* d3d, ID3D12GraphicsCommandList* cmdList)
{
    m_heap.SetHeap(cmdList);

    // GBuffer Pass
    {
        if (!m_deferredContext.IsInitialized())
            m_deferredContext.Init(d3d->GetDevice(), cmdList, &m_heapRTV, &m_heap);

        const Skybox* skybox = m_studioConfig.EnvMapEnabled ? &m_skybox : nullptr;
        m_deferredContext.RenderGBuffer(d3d, cmdList, m_camera.GetViewMatrix(), m_projMatrix, &m_heap, skybox);
    }

    // Lighting Pass
    {
        if (!m_denoisingManager.IsInitialized())
            m_denoisingManager.Init(d3d->GetDevice(), cmdList, &m_heap);

        m_deferredContext.RenderLighting(d3d, cmdList, &m_heap, m_projMatrix, &m_rtvPingPong1,
                                         m_studioConfig.DirLight.DirLightDirection, m_envMap.GetCubemap(),
                                         m_skybox.GetIrradianceMap(), m_texBrdfIntegrationMap.GetD12Resource(),
                                         m_studioConfig.Raster.Mode, &m_denoisingManager);
    }

    m_finalRTV = &m_rtvPingPong1;

    copyRtvTex(cmdList, d3d->GetRtv(), m_finalRTV->GetD12Resource());
}

void SceneStudio::ResetCameraToSceneStart()
{
    const auto currScene = m_sceneConfigs.at(m_currentScene);
    m_camera.GetCamera().SetPosition(currScene.InitialCamPos);
    m_camera.GetCamera().SetPitchYaw(currScene.InitialCamPitchYaw.x, currScene.InitialCamPitchYaw.y);
}