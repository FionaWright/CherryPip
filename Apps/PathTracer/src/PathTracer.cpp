#include "Apps/PathTracer/Headers/PathTracer.h"
#include "System/Win32App.h"
#include <dxcapi.h>

#include "imgui.h"
#include "Debug/GPUEventScoped.h"
#include "Headers/Helper.h"
#include "HWI/D3D.h"
#include "System/Config.h"
#include "System/FileHelper.h"
#include "System/Gui.h"
#include "CBV.h"
#include "Debug/PythonExecutor.h"
#include "HWI/BLAS.h"
#include "HWI/Material.h"
#include "../../../Headers/client/Render/Scene.h"
#include "System/Input.h"
#include "System/ModelLoaderGLTF.h"

PathTracer::PathTracer()
    : m_AspectRatio(0)
{
}

void PathTracer::OnInit(D3D* d3d)
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

    m_camera.Init({}, {});

    loadAssets(d3d);
}

void PathTracer::OnUpdate(D3D* d3d, ID3D12GraphicsCommandList* cmdList)
{
    if (!d3d->GetRayTracingSupported())
    {
        std::cout << "ERROR: Ray-Tracing not supported!!!" << std::endl;
        return;
    }

    populateCommandList(d3d, cmdList);
    const bool moved = m_camera.UpdateCamera();
    if (moved)
        m_ptContext.Reset();
}

void PathTracer::loadAssets(D3D* d3d)
{
    ID3D12Device* device = d3d->GetDevice();
    const ComPtr<ID3D12GraphicsCommandList> cmdList = d3d->GetAvailableCmdList(D3D12_COMMAND_LIST_TYPE_DIRECT);

    m_heap.Init(device, 10000, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    m_heapRTV.Init(device, 5, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

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
    args.Transform.SetScale(2.0f);
    ModelLoaderGLTF::LoadSplitModel(d3d, cmdList.Get(), &m_heap, L"Cornell/scene.gltf", args);

    std::shared_ptr<Scene> cornellScene = std::make_shared<Scene>();
    cornellScene->Init(args.OutObjects);
    m_scenes.emplace_back(cornellScene);

    m_ptContext.Init(device, cmdList.Get(), m_scenes.at(0).get());

    m_ptOutputTex.Init(L"PT Output", device, &m_heapRTV, Config::GetSystem().RtvWidth, Config::GetSystem().RtvHeight,
                       Config::GetSystem().RTVFormat);

    m_material = std::make_shared<Material>();
    m_material->Init(&m_heap);
    m_material->AddCBV(device, &m_heap, sizeof(CbvPathTracing));
    m_ptContext.FillMaterial(device, m_material.get(), &m_heap);

#ifdef _DEBUG
    m_rootSigDebug = std::make_shared<RootSig>();
    m_rootSigDebug->SmartInit(device, 2, 5, 2);

    m_materialDebug = std::make_shared<Material>();
    m_materialDebug->Init(&m_heap);
    m_materialDebug->AddCBV(device, &m_heap, sizeof(CbvPathTracing));
    m_materialDebug->AddCBV(device, &m_heap, sizeof(CbvPathTracingDebug));
    m_ptContext.FillMaterial(device, m_materialDebug.get(), &m_heap);

    m_readbackManager.Init(d3d, &m_heap, &m_ptOutputTex);
#endif

    V(cmdList->Close());
    d3d->ExecuteCommandList(cmdList.Get());
    d3d->Flush();
}

void PathTracer::populateCommandList(D3D* d3d, ID3D12GraphicsCommandList* cmdList)
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

    const bool debugMode = m_ptConfig.Mode == eOutputBuffer;

    ID3D12RootSignature* rootSig = debugMode ? m_rootSigDebug->Get() : m_rootSig->Get();
    const Material* mat = debugMode ? m_materialDebug.get() : m_material.get();
    const int debugBufferIdx = debugMode ? m_ptConfig.DebugBufferIdx : -1;
    PathTracingContext* ptContext = &m_ptContext;

    ID3D12PipelineState* pso = nullptr;
    switch (m_ptConfig.Mode)
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
        ptContext = m_furnaceTest.GetPtContext();
        pso = m_ptConfig.Mode == eFurnaceTestClassic ? m_furnaceTest.GetShaderClassic() : m_furnaceTest.GetShaderEmissive();
        mat = m_furnaceTest.GetMaterial();
        break;
    }

    ptContext->Render(cmdList, rootSig, pso, &m_camera.GetCamera(), mat, m_projMatrix, m_ptConfig, debugBufferIdx);

#ifdef _DEBUG
    if (m_ptConfig.ReadbackEnabled)
        m_readbackManager.ReadbackPass(d3d, cmdList, &m_ptOutputTex, m_ptConfig.ReadbackEveryFrame);
#endif

    D12Resource* rtv = d3d->GetRtv();
    m_ptOutputTex.GetD12Resource()->Transition(cmdList, D3D12_RESOURCE_STATE_COPY_SOURCE);
    rtv->Transition(cmdList, D3D12_RESOURCE_STATE_COPY_DEST);
    ID3D12Resource* srcResource = m_ptOutputTex.GetD12Resource()->GetResource();
    rtv->CopyTextureInto(cmdList, srcResource, Config::GetSystem().WindowAppGuiWidth);
    m_ptOutputTex.GetD12Resource()->Transition(cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);

    GUI();
}

