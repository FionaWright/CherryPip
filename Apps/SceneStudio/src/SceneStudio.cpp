#include "Apps/SceneStudio/Headers/SceneStudio.h"
#include "System/Win32App.h"
#include <dxcapi.h>

#include "ThirdParty/imgui/imgui.h"
#include "Debug/GPUEventScoped.h"
#include "Helper.h"
#include "HWI/D3D.h"
#include "System/Config.h"
#include "System/FileHelper.h"
#include "System/Gui.h"
#include "CBV.h"
#include "MathUtils.h"
#include "Debug/PythonExecutor.h"
#include "HWI/BLAS.h"
#include "HWI/Material.h"
#include "Render/Scene.h"
#include "System/Input.h"
#include "System/ModelLoaderGLTF.h"

void SceneStudio::OnInit(D3D* d3d)
{
    App::OnInit(d3d);

    if (!d3d->GetRayTracingSupported())
    {
        std::cout << "ERROR: Ray-Tracing not supported!!!" << std::endl;
        return;
    }

    m_AspectRatio = static_cast<float>(Config::GetSystem().RtvWidth) / static_cast<float>(Config::GetSystem().
        RtvHeight);

    constexpr float fov = 60.0f;
    constexpr float nearPlane = 0.1f;
    constexpr float farPlane = 100.0f;
    m_projMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(fov), m_AspectRatio, nearPlane, farPlane);

    m_camera.Init(XMFLOAT3(0, 0, 5), 0, PI);

    loadAssets(d3d);
}

void SceneStudio::OnUpdate(D3D* d3d, ID3D12GraphicsCommandList* cmdList)
{
    if (!d3d->GetRayTracingSupported())
    {
        std::cout << "ERROR: Ray-Tracing not supported!!!" << std::endl;
        return;
    }

    if (m_sceneDirty)
    {
        Scene* currScene = m_scenes.at(m_currentScene).get();
        m_ptContext.BuildScene(d3d->GetDevice(), cmdList, currScene, &m_heap);
        m_rasterContext.SetScene(currScene);

        m_camera.GetCamera().SetPosition(currScene->GetCameraPosition());
        m_camera.GetCamera().SetPitchYaw(currScene->GetPitch(), currScene->GetYaw());

        m_sceneDirty = false;
    }

    switch (m_studioConfig.Backend)
    {
    case eForward:
        renderRaster(d3d, cmdList);
        break;
    case ePathTracer:
        renderPathTracer(d3d, cmdList);
        break;
    default:
        break;
    }

    GUI();

    const bool moved = m_camera.UpdateCamera();
    if (moved)
        m_ptContext.Reset();
}

void SceneStudio::loadAssets(D3D* d3d)
{
    ID3D12Device* device = d3d->GetDevice();
    const ComPtr<ID3D12GraphicsCommandList> cmdList = d3d->GetAvailableCmdList(D3D12_COMMAND_LIST_TYPE_DIRECT);

    m_heap.Init(device, 10000, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    m_heapRTV.Init(device, 5, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    loadRasterAssets(d3d);

    m_rootSig = std::make_shared<RootSig>();
    m_rootSig->SmartInit(device, 1, 5, 1);

    // Init Shader/PSO
    {
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
        m_shader->InitVsPs(L"FullScreenTriangleVS.hlsl", L"Path-Tracing/PS.hlsl", ild, device, m_rootSig->Get());
    }

    GLTFLoadArgs args;
    args.Root = m_rootSigRaster;
    args.DefaultShaderIndex = 0;
    args.Shaders = { m_shaderRaster };

    args.Transform.SetScale(2.0f);
    ModelLoaderGLTF::LoadSplitModel(d3d, cmdList.Get(), &m_heap, L"Cornell/scene.gltf", args);
    std::shared_ptr<Scene> sceneCornellBox = std::make_shared<Scene>();
    sceneCornellBox->Init("Cornell Box", XMFLOAT3(0, 1.5f, 4.5f), 0, PI, args.OutObjects);
    m_scenes.emplace_back(sceneCornellBox);
    args.OutObjects.clear();
    //
    // ModelLoaderGLTF::LoadSplitModel(d3d, cmdList.Get(), &m_heap, L"Sphere/Sphere.gltf", args);
    // std::shared_ptr<Scene> sceneSphere = std::make_shared<Scene>();
    // sceneSphere->Init("Sphere", XMFLOAT3(0, 0, -4.3f), 0, 0, args.OutObjects);
    // m_scenes.emplace_back(sceneSphere);
    // args.OutObjects.clear();
    //
    // args.Transform = {};
    // args.Transform.SetScale(1.0f);
    // ModelLoaderGLTF::LoadSplitModel(d3d, cmdList.Get(), &m_heap, L"floatplane.glb", args);
    // std::shared_ptr<Scene> scenePlane = std::make_shared<Scene>();
    // scenePlane->Init("FloatPlane", XMFLOAT3(0, 1.5f, 4.5f), 0, PI, args.OutObjects);
    // m_scenes.emplace_back(scenePlane);
    // args.OutObjects.clear();

    // args.Transform.SetScale(2.0f);
    // ModelLoaderGLTF::LoadSplitModel(d3d, cmdList.Get(), &m_heap, L"Chess/Chess.gltf", args);
    // std::shared_ptr<Scene> sceneChess = std::make_shared<Scene>();
    // sceneChess->Init("Chess", {}, 0, 0, args.OutObjects);
    // m_scenes.emplace_back(sceneChess);
    // args.OutObjects.clear();

    m_ptContext.Init(device, cmdList.Get(), &m_heap);

    m_ptOutputTex.Init(L"PT Output", device, &m_heapRTV, Config::GetSystem().RtvWidth, Config::GetSystem().RtvHeight,
                       Config::GetSystem().RTVFormat);

#ifdef _DEBUG
    m_rootSigDebug = std::make_shared<RootSig>();
    m_rootSigDebug->SmartInit(device, 2, 5, 2);

    m_readbackManager.Init(d3d, &m_heap, &m_ptOutputTex);
#endif

    V(cmdList->Close());
    d3d->ExecuteCommandList(cmdList.Get());
    d3d->Flush();
}

void SceneStudio::loadRasterAssets(const D3D* d3d)
{
    D3D12_STATIC_SAMPLER_DESC samplers[1];
    samplers[0] = {};
    samplers[0].Filter = D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
    samplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    samplers[0].ShaderRegister = 0;

    m_rootSigRaster = std::make_shared<RootSig>();
    m_rootSigRaster->SmartInit(d3d->GetDevice(), 1, 1, 0, samplers, _countof(samplers));

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
    m_shaderRaster->InitVsPs(L"Basic3D_GltfVS.hlsl", L"Basic3D_GltfPS.hlsl", {rasterILD, _countof(rasterILD)}, d3d->GetDevice(), m_rootSigRaster->Get());
}

void SceneStudio::renderPathTracer(D3D* d3d, ID3D12GraphicsCommandList* cmdList)
{
    const float fRtvWidth = static_cast<float>(Config::GetSystem().RtvWidth);
    const float fRtvHeight = static_cast<float>(Config::GetSystem().RtvHeight);

    const CD3DX12_VIEWPORT viewport(0.0f, 0.0f, fRtvWidth, fRtvHeight);
    const CD3DX12_RECT scissorRect(0, 0, Config::GetSystem().RtvWidth, Config::GetSystem().RtvHeight);

    cmdList->RSSetViewports(1, &viewport);
    cmdList->RSSetScissorRects(1, &scissorRect);

    m_ptOutputTex.GetD12Resource()->Transition(cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);
    const UINT rtvIdx = m_ptOutputTex.GetHeapIdx();
    const auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_heapRTV.GetCPUHandle(), rtvIdx, m_heapRTV.GetIncrementSize());
    cmdList->OMSetRenderTargets(1, &handle, FALSE, nullptr);

    m_heap.SetHeap(cmdList);

    const bool debugMode = m_studioConfig.PT.Mode == eOutputBuffer;

    ID3D12RootSignature* rootSig = debugMode ? m_rootSigDebug->Get() : m_rootSig->Get();
    const int debugBufferIdx = debugMode ? m_studioConfig.PT.DebugBufferIdx : -1;

    ID3D12PipelineState* pso = nullptr;
    switch (m_studioConfig.PT.Mode)
    {
    case eStandard:
        pso = m_shader->GetPSO();
        break;
    case eOutputBuffer:
        if (!m_shaderDebug)
        {
            const D3D12_INPUT_LAYOUT_DESC ild = {m_shaderILD.data(), static_cast<UINT>(m_shaderILD.size())};
            m_shaderDebug = std::make_shared<Shader>();
            m_shaderDebug->InitVsPs(L"FullScreenTriangleVS.hlsl", L"Path-Tracing/DebugPS.hlsl", ild, d3d->GetDevice(),
                                    m_rootSigDebug->Get());
        }
        pso = m_shaderDebug->GetPSO();
        break;
    case eFurnaceTestClassic:
    case eFurnaceTestEmissive:
        if (!m_furnaceTest.GetIsInitialised())
            m_furnaceTest.Init(d3d, cmdList, m_shaderILD, m_rootSig->Get(), &m_heap);
        pso = m_studioConfig.PT.Mode == eFurnaceTestClassic ? m_furnaceTest.GetShaderClassic() : m_furnaceTest.GetShaderEmissive();
        break;
    }

    m_ptContext.Render(cmdList, rootSig, pso, &m_camera.GetCamera(), m_projMatrix, m_studioConfig.PT, debugBufferIdx);

#ifdef _DEBUG
    if (m_studioConfig.PT.ReadbackEnabled)
        m_readbackManager.ReadbackPass(d3d, cmdList, &m_ptOutputTex, m_studioConfig.PT.ReadbackEveryFrame);
#endif

    D12Resource* rtv = d3d->GetRtv();
    m_ptOutputTex.GetD12Resource()->Transition(cmdList, D3D12_RESOURCE_STATE_COPY_SOURCE);
    rtv->Transition(cmdList, D3D12_RESOURCE_STATE_COPY_DEST);
    ID3D12Resource* srcResource = m_ptOutputTex.GetD12Resource()->GetResource();
    rtv->CopyTextureInto(cmdList, srcResource, Config::GetSystem().WindowAppGuiWidth);
    m_ptOutputTex.GetD12Resource()->Transition(cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);
}

void SceneStudio::renderRaster(const D3D* d3d, ID3D12GraphicsCommandList* cmdList) const
{
    m_heap.SetHeap(cmdList);

    m_rasterContext.Render(d3d, cmdList, m_camera.GetViewMatrix(), m_projMatrix);
}
