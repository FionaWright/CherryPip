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
#include "HWI/Material.h"
#include "System/ModelLoaderGLTF.h"

PathTracer::PathTracer()
    : m_AspectRatio(0)
{
}

void PathTracer::OnInit(D3D* d3d)
{
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
    m_camera.UpdateCamera();
}

void PathTracer::loadAssets(D3D* d3d)
{
    ID3D12Device* device = d3d->GetDevice();
    const ComPtr<ID3D12GraphicsCommandList> cmdList = d3d->GetAvailableCmdList(D3D12_COMMAND_LIST_TYPE_DIRECT);

    m_shader = std::make_shared<Shader>();
    m_heap.Init(device, 10000);

    m_rootSig = std::make_shared<RootSig>();
    m_rootSig->SmartInit(device, 1, 5);

    // Init Shader/PSO
    {
        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
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
        const D3D12_INPUT_LAYOUT_DESC ild = {inputElementDescs, _countof(inputElementDescs)};

        m_shader->InitVsPs(L"FullScreenTriangleVS.hlsl", L"Path-Tracing/CorePS.hlsl", ild, device, m_rootSig->Get());
    }

    std::shared_ptr<Texture> tex = std::make_shared<Texture>();
    tex->Init(d3d->GetDevice(), cmdList.Get(), FileHelper::GetAssetTextureFullPath(L"TestTex.png"),
              DXGI_FORMAT_R8G8B8A8_UNORM, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

    tex->Transition(cmdList.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    GLTFLoadArgs args;
    args.Transform = {};
    args.Transform.SetScale(0.1f);
    args.DefaultShaderIndex = 0;
    args.DefaultShaderATIndex = -1;
    args.ExportBlasModeEnabled = true;
    ModelLoaderGLTF::LoadSplitModel(d3d, cmdList.Get(), &m_heap, L"floatplane.glb", args);
    args.Transform = {};
    args.Transform.SetPosition(0, 10, 0);
    args.Transform.SetScale(0.1f);
    ModelLoaderGLTF::LoadSplitModel(d3d, cmdList.Get(), &m_heap, L"floatplane.glb", args);
    args.Transform = {};
    args.Transform.SetPosition(0, 5, 0);
    args.Transform.SetScale(0.01f);
    ModelLoaderGLTF::LoadSplitModel(d3d, cmdList.Get(), &m_heap, L"floatplane.glb", args);
    args.Transform = {};
    args.Transform.SetPosition(0, 0, -500);
    args.Transform.SetRotation(90, 0, 0);
    args.Transform.SetScale(10.0f);
    ModelLoaderGLTF::LoadSplitModel(d3d, cmdList.Get(), &m_heap, L"floatplane.glb", args);
    auto blasList = args.BLASs;


    ComPtr<ID3D12Device5> device5;
    V(d3d->GetDevice()->QueryInterface(IID_PPV_ARGS(&device5)));
    ComPtr<ID3D12GraphicsCommandList4> cmdList4;
    V(cmdList->QueryInterface(IID_PPV_ARGS(&cmdList4)));
    auto tlas = std::make_shared<TLAS>();
    tlas->Init(device5.Get(), cmdList4.Get(), blasList);

    m_ptContext.Init(device, cmdList.Get(), tlas, blasList);

    m_material = std::make_shared<Material>();
    m_material->Init(&m_heap);
    m_material->AddCBV(device, &m_heap, sizeof(CbvPathTracing));
    m_ptContext.FillMaterial(device,  m_material.get(), &m_heap);

    V(cmdList->Close());
    d3d->ExecuteCommandList(cmdList.Get());
    d3d->Flush();
}

void PathTracer::populateCommandList(const D3D* d3d, ID3D12GraphicsCommandList* cmdList) const
{
    const float fRtvWidth = static_cast<float>(Config::GetSystem().RtvWidth);
    const float fRtvHeight = static_cast<float>(Config::GetSystem().RtvHeight);
    const float fAppGuiWidth = static_cast<float>(Config::GetSystem().WindowAppGuiWidth);

    // Render at offset for ImGui
    const CD3DX12_VIEWPORT viewport(fAppGuiWidth, 0.0f, fRtvWidth, fRtvHeight);
    const CD3DX12_RECT scissorRect(Config::GetSystem().WindowAppGuiWidth, 0,
                                   Config::GetSystem().RtvWidth + Config::GetSystem().WindowAppGuiWidth,
                                   Config::GetSystem().RtvHeight);

    cmdList->RSSetViewports(1, &viewport);
    cmdList->RSSetScissorRects(1, &scissorRect);

    m_heap.SetHeap(cmdList);

    // Note: HERE AS WELL
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(d3d->GetRtvHeapStart(), d3d->GetFrameIndex(), d3d->GetRtvDescriptorSize());
    cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    constexpr float clearColor[] = {0.0f, 0.2f, 0.4f, 1.0f};
    cmdList->ClearRenderTargetView(rtvHandle, clearColor, 1, &scissorRect);

    m_ptContext.Render(d3d->GetDevice(), cmdList, m_rootSig->Get(), m_shader->GetPSO(), &m_camera.GetCamera(), m_material.get(), m_projMatrix);

    {
        Gui::BeginWindow("PathTracer", ImVec2(0, 0), ImVec2(Config::GetSystem().WindowAppGuiWidth, Config::GetSystem().RtvHeight));
        ImGui::Text("APP-SIDE GUI (PathTracer)");
        Gui::EndWindow();
    }
}
