//
// Created by fiona on 15/01/2026.
//

#include "Render/DeferredContext.h"

#include "CBV.h"
#include "Render/Object.h"
#include "Render/Skybox.h"
#include "System/Config.h"

#include "Debug/GPUEventScoped.h"

void DeferredContext::Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, Heap* heap)
{
    // Could improve precision here for increased bandwidth
    m_rtvAlbedo.Init(L"Albedo GBuffer", device, heap, Config::GetSystem().RtvWidth, Config::GetSystem().RtvHeight, DXGI_FORMAT_R8G8B8A8_UNORM);
    m_rtvNormalsDepth.Init(L"Normals GBuffer", device, heap, Config::GetSystem().RtvWidth, Config::GetSystem().RtvHeight, DXGI_FORMAT_R8G8B8A8_UNORM);

    m_rtvHandles = {
        m_rtvAlbedo.GetCpuHandle(),
        m_rtvNormalsDepth.GetCpuHandle()
    };

    D3D12_STATIC_SAMPLER_DESC samplers[1];
    samplers[0] = {};
    samplers[0].Filter = D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
    samplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    samplers[0].ShaderRegister = 0;

    m_rootSigGBuffer.SmartInit(device, 1, 1, 0, false, samplers, _countof(samplers));

    D3D12_INPUT_ELEMENT_DESC ildDesc[] =
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
    m_shaderGBuffer.InitVsPs(L"Raster/Deferred/GBufferVS.hlsl", L"Raster/Deferred/GBufferPS.hlsl", {ildDesc, _countof(ildDesc)}, device, m_rootSigGBuffer.Get(), true, {}, m_rtvHandles.size());
}

void DeferredContext::SetScene(Scene* scene)
{
    m_scene = scene;
}

void DeferredContext::Render(const D3D* d3d, ID3D12GraphicsCommandList* cmdList, const XMMATRIX& vMatrix, const XMMATRIX& pMatrix, const Skybox* skybox)
{
    GPU_SCOPE(cmdList, L"Deferred Backend");

    assert(m_scene);

    const float fRtvWidth = static_cast<float>(Config::GetSystem().RtvWidth);
    const float fRtvHeight = static_cast<float>(Config::GetSystem().RtvHeight);

    const CD3DX12_VIEWPORT viewport(0.0f, 0.0f, fRtvWidth, fRtvHeight);
    const CD3DX12_RECT scissorRect(0, 0, Config::GetSystem().RtvWidth, Config::GetSystem().RtvHeight);

    cmdList->RSSetViewports(1, &viewport);
    cmdList->RSSetScissorRects(1, &scissorRect);

    m_rtvAlbedo.GetD12Resource()->Transition(cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_rtvNormalsDepth.GetD12Resource()->Transition(cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);

    const CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(d3d->GetDsvHeapStart(), d3d->GetFrameIndex(), d3d->GetDsvDescriptorSize());
    cmdList->OMSetRenderTargets(m_rtvHandles.size(), m_rtvHandles.data(), FALSE, &dsvHandle);

    for (const auto rtvHandle : m_rtvHandles)
        cmdList->ClearRenderTargetView(rtvHandle, Config::GetSystem().RtvClearColor, 1, &scissorRect);
    cmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    CbvMatrices matrices = {};
    matrices.V = vMatrix;
    matrices.P = pMatrix;

    cmdList->SetGraphicsRootSignature(m_rootSigGBuffer.Get());
    cmdList->SetPipelineState(m_shaderGBuffer.GetPSO());

    const auto& objects = m_scene->GetObjects();
    for (const auto & object : objects)
    {
        GPU_SCOPE(cmdList, object->GetName());

        const Transform* transform = object->GetTransform();
        const auto model = object->GetModel();
        const auto mat = object->GetMaterial();

        matrices.M = transform->GetModelMatrix();
        matrices.MTI = XMMatrixInverse(nullptr, XMMatrixTranspose(matrices.M));
        mat->UpdateCBV(0, &matrices);

        mat->TransitionSrvsToPS(cmdList);
        mat->SetDescriptorTables(cmdList);

        cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        cmdList->IASetVertexBuffers(0, 1, &model->GetVertexBufferView());
        cmdList->IASetIndexBuffer(&model->GetIndexBufferView());
        cmdList->DrawIndexedInstanced(static_cast<UINT>(model->GetIndexCount()), 1, 0, 0, 0);
    }

    // Skybox into Albedo pass
    {
        GPU_SCOPE(cmdList, "Skybox -> GBuffer");

        const auto albedoHandle = m_rtvAlbedo.GetCpuHandle();
        cmdList->OMSetRenderTargets(1, &albedoHandle, FALSE, &dsvHandle);

        if (skybox)
            skybox->RenderForward(d3d, cmdList, vMatrix, pMatrix);
    }
}
