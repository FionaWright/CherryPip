//
// Created by fionaw on 28/10/2025.
//

#include "Render/PathTracingContext.h"

#include <d3dx12_core.h>

#include "CBV.h"
#include "Helper.h"
#include "Apps/PathTracer/Headers/PathTracer.h"
#include "Debug/GPUEventScoped.h"
#include "HWI/BLAS.h"
#include "HWI/Material.h"
#include "Render/Camera.h"
#include "System/Config.h"

void PathTracingContext::Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList,
                              const std::shared_ptr<TLAS>& tlas, const std::vector<std::shared_ptr<BLAS>>& blasList,
                              Heap* heap, D12Resource* rtv)
{
    m_tlas = tlas;
    m_blasList = blasList;

    // Temp
    const std::vector<PtMaterialData> materials = {
        {XMFLOAT3(1, 0.2, 0.2), 0.0f},
        {XMFLOAT3(0.2, 1.0, 0.2), 0.0f},
        {XMFLOAT3(1, 1, 1), 10.0f},
        {XMFLOAT3(1, 1, 0), 0.3f},
    };

    UINT curVertexBufferOffset = 0;
    UINT curIndexBufferOffset = 0;
    for (int i = 0; i < m_blasList.size(); i++)
    {
        const Model* model = m_blasList[i]->GetModel();

        PtInstanceData instanceData;
        instanceData.VertexBufferOffset = curVertexBufferOffset;
        instanceData.IndexBufferOffset = curIndexBufferOffset;
        instanceData.MaterialIdx = i;
        instanceData.MTI = XMMatrixTranspose(XMMatrixInverse(nullptr, m_blasList[i]->GetTransform().GetModelMatrix()));
        m_instanceDataList.emplace_back(instanceData);

        curVertexBufferOffset += model->GetVertexCount();
        curIndexBufferOffset += model->GetIndexCount() / 3;
    }

    m_vertexMegaBufferCount = curVertexBufferOffset;
    m_indexMegaBufferCount = curIndexBufferOffset;

    const UINT64 vMegaBufferSize = sizeof(Vertex) * m_vertexMegaBufferCount;
    m_vertexMegaBuffer = std::make_shared<D12Resource>();
    m_vertexMegaBuffer->InitBuffer(L"Path-Tracing Vertex Mega Buffer", device, vMegaBufferSize,
                             D3D12_RESOURCE_STATE_COPY_DEST);

    const UINT64 iMegaBufferSize = sizeof(uint32_t) * 3 * m_indexMegaBufferCount;
    m_indexMegaBuffer = std::make_shared<D12Resource>();
    m_indexMegaBuffer->InitBuffer(L"Path-Tracing Index Mega Buffer", device, iMegaBufferSize, D3D12_RESOURCE_STATE_COPY_DEST);

    const UINT64 bufferSize = sizeof(PtInstanceData) * m_instanceDataList.size();
    m_instanceDataBuffer = std::make_shared<D12Resource>();
    m_instanceDataBuffer->InitBuffer(L"Path-Tracing Instance Data Buffer", device, bufferSize, D3D12_RESOURCE_STATE_COMMON);
    m_instanceDataBuffer->UploadData(device, cmdList, m_instanceDataList.data(), bufferSize);

    const UINT64 mBufferSize = sizeof(PtMaterialData) * m_instanceDataList.size();
    m_materialBuffer = std::make_shared<D12Resource>();
    m_materialBuffer->InitBuffer(L"Path-Tracing Material Data Buffer", device, mBufferSize, D3D12_RESOURCE_STATE_COMMON);
    m_materialBuffer->UploadData(device, cmdList, materials.data(), mBufferSize);

    size_t vByteOffset = 0;
    size_t iByteOffset = 0;
    for (int i = 0; i < m_blasList.size(); i++)
    {
        const Model* model = m_blasList[i]->GetModel();
        const auto vBuffer = model->GetVertexBuffer();
        const auto iBuffer = model->GetIndexBuffer();

        vBuffer->Transition(cmdList, D3D12_RESOURCE_STATE_COPY_SOURCE);
        iBuffer->Transition(cmdList, D3D12_RESOURCE_STATE_COPY_SOURCE);

        const UINT vBufferSize = sizeof(Vertex) * model->GetVertexCount();
        cmdList->CopyBufferRegion(m_vertexMegaBuffer->GetResource(), vByteOffset, vBuffer->GetResource(), 0,
                                  vBufferSize);

        const UINT iBufferSize = sizeof(uint32_t) * model->GetIndexCount();
        cmdList->CopyBufferRegion(m_indexMegaBuffer->GetResource(), iByteOffset, iBuffer->GetResource(), 0,
                                  iBufferSize);

        vByteOffset += vBufferSize;
        iByteOffset += iBufferSize;
    }

    m_fullScreenTriangle.InitFullScreenTriangle(device, cmdList);

    {
        m_pathTraceOutTexture = std::make_shared<Texture>();
        m_pathTraceOutTexture->InitEmpty(device, Config::GetSystem().RTVFormat, Config::GetSystem().RtvWidth,
                                         Config::GetSystem().RtvHeight, 1,
                                         D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

        // TODO: Encapsulate?
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = 1; // or more if you have multiple RTVs
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        V(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format = m_pathTraceOutTexture->GetFormat();
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Texture2D.MipSlice = 0;
        rtvDesc.Texture2D.PlaneSlice = 0;

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
        device->CreateRenderTargetView(m_pathTraceOutTexture->GetD12Resource()->GetResource(), &rtvDesc, rtvHandle);
    }

    m_accumTexture = std::make_shared<Texture>();
    m_accumTexture->InitEmpty(device, Config::GetSystem().RTVFormat, Config::GetSystem().RtvWidth, Config::GetSystem().RtvHeight, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

    D3D12_STATIC_SAMPLER_DESC samplers[1];
    samplers[0] = {};
    samplers[0].Filter = D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
    samplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    samplers[0].ShaderRegister = 0;

    m_accumRootSig = std::make_shared<RootSig>();
    m_accumRootSig->SmartInit(device, 1, 1, 2, samplers, _countof(samplers));

    m_accumMaterial = std::make_shared<Material>();
    m_accumMaterial->Init(heap);
    m_accumMaterial->AddCBV(device, heap, sizeof(CbvAccum));
    m_accumMaterial->AddSRV(device, heap, m_pathTraceOutTexture);
    m_accumMaterial->AddUAV(device, heap, m_accumTexture);
    m_accumMaterial->AddUAV(device, heap, rtv->GetResource(), Config::GetSystem().RTVFormat);

    m_accumShader = std::make_shared<Shader>();
    m_accumShader->InitCs(L"Path-Tracing/AccumCS.hlsl", device, m_accumRootSig->Get());

    m_rng = std::mt19937(INITIAL_SEED);
    m_rngDist = std::uniform_int_distribution<UINT>(0, UINT32_MAX);
}

void PathTracingContext::FillMaterial(ID3D12Device* device, Material* material, Heap* heap) const
{
    material->AddTLAS(device, heap, m_tlas);
    material->AddBuffer(device, heap, m_instanceDataBuffer, m_instanceDataList.size(), sizeof(PtInstanceData));
    material->AddBuffer(device, heap, m_vertexMegaBuffer, m_vertexMegaBufferCount, sizeof(Vertex));
    material->AddBuffer(device, heap, m_indexMegaBuffer, m_indexMegaBufferCount, sizeof(uint32_t) * 3);
    material->AddBuffer(device, heap, m_materialBuffer, m_instanceDataList.size(), sizeof(PtMaterialData));
}

void PathTracingContext::Render(ID3D12GraphicsCommandList* cmdList, ID3D12RootSignature* rootSig, D12Resource* rtv,
                                ID3D12PipelineState* pso, const Camera* camera, const Material* material,
                                const XMMATRIX& projMatrix, const PtConfig& config)
{
    GPU_SCOPE(cmdList, L"Path Tracing");

    if (!config.RngPaused)
        m_curRngState = m_rngDist(m_rng);

    {
        cmdList->SetGraphicsRootSignature(rootSig);
        cmdList->SetPipelineState(pso);

        m_pathTraceOutTexture->Transition(cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);

        const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
        cmdList->OMSetRenderTargets(1, &rtvHandle, false, nullptr);

        CbvPathTracing cbv;
        cbv.CameraPositionWorld = camera->GetPosition();
        cbv.InvP = XMMatrixInverse(nullptr, projMatrix);
        cbv.InvV = XMMatrixInverse(nullptr, camera->GetViewMatrix());
        cbv.NumBounces = 2;
        cbv.Seed = m_curRngState;
        cbv.SPP = config.SPP;

        material->UpdateCBV(0, &cbv);
        material->SetDescriptorTables(cmdList);

        cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        cmdList->IASetVertexBuffers(0, 1, &m_fullScreenTriangle.GetVertexBufferView());
        cmdList->IASetIndexBuffer(&m_fullScreenTriangle.GetIndexBufferView());
        cmdList->DrawIndexedInstanced(static_cast<UINT>(m_fullScreenTriangle.GetIndexCount()), 1, 0, 0, 0);
    }

    {
        m_pathTraceOutTexture->GetD12Resource()->Transition(cmdList, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE|D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        m_accumTexture->Transition(cmdList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        rtv->Transition(cmdList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

        cmdList->SetComputeRootSignature(m_accumRootSig->Get());
        cmdList->SetPipelineState(m_accumShader->GetPSO());

        CbvAccum accum;
        accum.NumFrames = m_numFrames;
        accum.TexelSize.x = 1.0f / Config::GetSystem().RtvWidth;
        accum.TexelSize.y = 1.0f / Config::GetSystem().RtvHeight;

        m_accumMaterial->UpdateCBV(0, &accum);
        m_accumMaterial->SetDescriptorTables(cmdList);

        const uint32_t groupX = (Config::GetSystem().RtvWidth + 7) / 8;
        const uint32_t groupY = (Config::GetSystem().RtvHeight + 7) / 8;
        cmdList->Dispatch(groupX, groupY, 1);

        rtv->Transition(cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);
    }

    m_numFrames++;
}

void PathTracingContext::Reset()
{
    m_numFrames = 0;
    // TODO: Clear accum
}
