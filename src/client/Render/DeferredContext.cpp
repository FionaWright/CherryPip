//
// Created by fiona on 15/01/2026.
//

#include "System/pch.h"
#include "Render/DeferredContext.h"
#include "CBV.h"
#include "Render/Object.h"
#include "Render/Skybox.h"
#include "System/Config.h"

#include "Debug/GPUEventScoped.h"

void DeferredContext::Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, Heap* heapRTV, Heap* heap)
{
    // Could improve precision here for increased bandwidth
    m_rtvAlbedo.Init(L"Albedo GBuffer", device, heapRTV, Config::GetSystem().RtvWidth, Config::GetSystem().RtvHeight, DXGI_FORMAT_R8G8B8A8_UNORM);
    m_rtvNormalsDepth.Init(L"Normals GBuffer", device, heapRTV, Config::GetSystem().RtvWidth, Config::GetSystem().RtvHeight, DXGI_FORMAT_R8G8B8A8_UNORM);
    m_rtvRoughMetEmissive.Init(L"Roughness Metallic Emissive Buffer", device, heapRTV, Config::GetSystem().RtvWidth, Config::GetSystem().RtvHeight, DXGI_FORMAT_R8G8B8A8_UNORM);

    m_rtvHandles = {
        m_rtvAlbedo.GetCpuHandle(),
        m_rtvNormalsDepth.GetCpuHandle(),
        m_rtvRoughMetEmissive.GetCpuHandle(),
    };

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

    m_rootSigGBuffer.SmartInit(device, 1, 4, 0, false, samplers, _countof(samplers));
    m_rootSigLighting.SmartInit(device, 2, 6, 0, false, samplers, _countof(samplers));

    {
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

    {
        D3D12_INPUT_ELEMENT_DESC ildDesc[] =
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

        m_shaderLighting.InitVsPs(L"FullScreenTriangleVS.hlsl", L"Raster/Deferred/DeferredPS.hlsl", {ildDesc, _countof(ildDesc)}, device, m_rootSigLighting.Get(), false);
    }

    m_matLighting.Init(heap);
    m_matLighting.AddCBV(device, heap, sizeof(CbvDeferredLighting));
    m_matLighting.AddCBV(device, heap, sizeof(CbvRasterDebug));

    m_fullScreenTriangle.InitFullScreenTriangle(device, cmdList);

    m_initialized = true;
}

void DeferredContext::SetScene(Scene* scene)
{
    m_scene = scene;
}

void DeferredContext::RenderGBuffer(const D3D* d3d, ID3D12GraphicsCommandList* cmdList, const XMMATRIX& vMatrix, const XMMATRIX& pMatrix, const Skybox* skybox)
{
    GPU_SCOPE(cmdList, L"Deferred GBuffer Pass");

    assert(m_scene);

    m_rtvAlbedo.GetD12Resource()->Transition(cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_rtvNormalsDepth.GetD12Resource()->Transition(cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_rtvRoughMetEmissive.GetD12Resource()->Transition(cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);

    // Set RTVs
    {
        const float fRtvWidth = static_cast<float>(Config::GetSystem().RtvWidth);
        const float fRtvHeight = static_cast<float>(Config::GetSystem().RtvHeight);

        const CD3DX12_VIEWPORT viewport(0.0f, 0.0f, fRtvWidth, fRtvHeight);
        const CD3DX12_RECT scissorRect(0, 0, Config::GetSystem().RtvWidth, Config::GetSystem().RtvHeight);

        cmdList->RSSetViewports(1, &viewport);
        cmdList->RSSetScissorRects(1, &scissorRect);

        const CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(d3d->GetDsvHeapStart(), d3d->GetFrameIndex(), d3d->GetDsvDescriptorSize());
        cmdList->OMSetRenderTargets(m_rtvHandles.size(), m_rtvHandles.data(), FALSE, &dsvHandle);

        for (const auto rtvHandle : m_rtvHandles)
            cmdList->ClearRenderTargetView(rtvHandle, Config::GetRender().RtvClearColor, 1, &scissorRect);
        cmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
    }

    CbvMatrices matrices = {};
    XMStoreFloat4x4(&matrices.V, vMatrix);
    XMStoreFloat4x4(&matrices.P, pMatrix);

    cmdList->SetGraphicsRootSignature(m_rootSigGBuffer.Get());
    cmdList->SetPipelineState(m_shaderGBuffer.GetPSO());

    // Render Objects
    const auto& objects = m_scene->GetObjects();
    for (const auto & object : objects)
    {
        GPU_SCOPE(cmdList, object->GetName());

        const Transform* transform = object->GetTransform();
        const auto model = object->GetModel();
        const auto mat = object->GetMaterial();

        XMMATRIX M = transform->GetModelMatrix();
        XMStoreFloat4x4(&matrices.M, M);
        XMStoreFloat4x4(&matrices.MTI, XMMatrixInverse(nullptr, XMMatrixTranspose(M)));
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
        const CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(d3d->GetDsvHeapStart(), d3d->GetFrameIndex(), d3d->GetDsvDescriptorSize());
        cmdList->OMSetRenderTargets(1, &albedoHandle, FALSE, &dsvHandle);

        if (skybox)
            skybox->RenderForward(d3d, cmdList, vMatrix, pMatrix);
    }
}

void DeferredContext::RenderLighting(const D3D* d3d, ID3D12GraphicsCommandList* cmdList, Heap* heap, const XMMATRIX& pMatrix, TextureRTV* output, const XMFLOAT3& dirLightDir, D12Resource* skybox, D12Resource* irradianceMap, D12Resource* brdfIntegrationMap, const RasterDebugMode debugMode)
{
    GPU_SCOPE(cmdList, "Deferred Lighting Pass");

    output->GetD12Resource()->Transition(cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);
    const auto handle = output->GetCpuHandle();
    cmdList->OMSetRenderTargets(1, &handle, FALSE, nullptr);

    cmdList->SetGraphicsRootSignature(m_rootSigLighting.Get());
    cmdList->SetPipelineState(m_shaderLighting.GetPSO());

    CbvDeferredLighting cbv;
    cbv.DirLightDir = dirLightDir;
    cbv.MaxCubemapMipMaps = skybox->GetDesc().MipLevels;
    XMStoreFloat4x4(&cbv.InvP, XMMatrixInverse(nullptr, pMatrix));
    m_matLighting.UpdateCBV(0, &cbv);

    CbvRasterDebug cbvDebug;
    cbvDebug.Mode = debugMode;
    m_matLighting.UpdateCBV(1, &cbvDebug);

    m_matLighting.SetTex(d3d->GetDevice(), 0, heap, m_rtvAlbedo.GetD12Resource());
    m_matLighting.SetTex(d3d->GetDevice(), 1, heap, m_rtvNormalsDepth.GetD12Resource());
    m_matLighting.SetTex(d3d->GetDevice(), 2, heap, m_rtvRoughMetEmissive.GetD12Resource());

    m_matLighting.SetTex(d3d->GetDevice(), 3, heap, brdfIntegrationMap);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.Format = skybox->GetDesc().Format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.TextureCube.MipLevels = skybox->GetDesc().MipLevels;
    srvDesc.TextureCube.MostDetailedMip = 0;
    m_matLighting.SetSRV(d3d->GetDevice(), 4, heap, skybox, srvDesc);

    srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.Format = irradianceMap->GetDesc().Format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.TextureCube.MipLevels = 1;
    srvDesc.TextureCube.MostDetailedMip = 0;
    m_matLighting.SetSRV(d3d->GetDevice(), 5, heap, irradianceMap, srvDesc);

    m_matLighting.TransitionSrvsToPS(cmdList);
    m_matLighting.SetDescriptorTables(cmdList);

    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmdList->IASetVertexBuffers(0, 1, &m_fullScreenTriangle.GetVertexBufferView());
    cmdList->IASetIndexBuffer(&m_fullScreenTriangle.GetIndexBufferView());
    cmdList->DrawIndexedInstanced(static_cast<UINT>(m_fullScreenTriangle.GetIndexCount()), 1, 0, 0, 0);
}
