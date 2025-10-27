#include "Apps/RasterViewer/Headers/RasterViewer.h"
#include "System/Win32App.h"
#include <dxcapi.h>

#include "imgui.h"
#include "Headers/Helper.h"
#include "HWI/D3D.h"
#include "System/Config.h"
#include "System/FileHelper.h"
#include "System/Gui.h"
#include "CBV.h"
#include "HWI/Material.h"
#include "System/ModelLoaderGLTF.h"

RasterViewer::RasterViewer()
    : m_AspectRatio(0)
{
}

void RasterViewer::OnInit(D3D* d3d)
{
    m_AspectRatio = static_cast<float>(Config::GetSystem().RtvWidth) / static_cast<float>(Config::GetSystem().
        RtvHeight);

    m_camera.Init({}, {});

    loadAssets(d3d);
}

void RasterViewer::OnUpdate(D3D* d3d, ID3D12GraphicsCommandList* cmdList)
{
    populateCommandList(d3d, cmdList);
    m_camera.UpdateCamera();
}

struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT2 uv;
    XMFLOAT3 normal;
    XMFLOAT4 tangent;
};

void RasterViewer::loadAssets(D3D* d3d)
{
    ID3D12Device* device = d3d->GetDevice();
    const ComPtr<ID3D12GraphicsCommandList> cmdList = d3d->GetAvailableCmdList(D3D12_COMMAND_LIST_TYPE_DIRECT);

    std::shared_ptr<RootSig> rootSig = std::make_shared<RootSig>();
    std::shared_ptr<Shader> shader = std::make_shared<Shader>();

    m_heap.Init(device, 10000);

    // Init Root Sig
    {
        CD3DX12_ROOT_PARAMETER1 params[2];
        CD3DX12_DESCRIPTOR_RANGE1 ranges[2];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
        params[0].InitAsDescriptorTable(1, &ranges[0]);
        ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
        params[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);

        D3D12_STATIC_SAMPLER_DESC samplers[1];
        samplers[0] = {};
        samplers[0].Filter = D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        samplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        samplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        samplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        samplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        samplers[0].ShaderRegister = 0;

        rootSig->Init(params, _countof(params), samplers, _countof(samplers), device);
    }

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
        D3D12_INPUT_LAYOUT_DESC ild = {inputElementDescs, _countof(inputElementDescs)};

        shader->InitVsPs(L"Basic3D_GltfVS.hlsl", L"Basic3D_GltfPS.hlsl", ild, device, rootSig->Get());
    }

    std::shared_ptr<Texture> tex = std::make_shared<Texture>();
    tex->Init(d3d->GetDevice(), cmdList.Get(), FileHelper::GetAssetTextureFullPath(L"TestTex.png"),
              DXGI_FORMAT_R8G8B8A8_UNORM, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

    tex->Transition(cmdList.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    GLTFLoadArgs args;
    args.Transform = {};
    args.Transform.SetScale(0.1f);
    args.CullingWhiteList = {};
    args.DefaultShaderIndex = 0;
    args.DefaultShaderATIndex = -1;
    args.Overrides = {};
    args.Root = rootSig;
    args.Shaders = {shader};
    ModelLoaderGLTF::LoadSplitModel(d3d, cmdList.Get(), &m_heap, L"floatplane.glb", args);
    //ModelLoaderGLTF::LoadSplitModel(d3d, cmdList.Get(), &m_heap, L"Bistro/Bistro.gltf", args);
    //ModelLoaderGLTF::LoadSplitModel(d3d, cmdList.Get(), &m_heap, L"Cube.glb", args);
    m_objects = args.Objects;

    V(cmdList->Close());
    d3d->ExecuteCommandList(cmdList.Get());
    d3d->Flush();
}

void RasterViewer::populateCommandList(const D3D* d3d, ID3D12GraphicsCommandList* cmdList) const
{
    float fov = 60.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
    CbvMatrices matrices = {};
    matrices.V = m_camera.GetViewMatrix();
    matrices.P = XMMatrixPerspectiveFovLH(XMConvertToRadians(fov), m_AspectRatio, nearPlane, farPlane);

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

    for (int i = 0; i < m_objects.size(); ++i)
    {
        m_objects[i]->Render(cmdList, matrices);
    }

    Gui::BeginWindow("App", ImVec2(0,0), ImVec2(Config::GetSystem().WindowAppGuiWidth, Config::GetSystem().RtvHeight));
    ImGui::Text("APP-SIDE GUI (RasterViewer)");
    Gui::EndWindow();
}
