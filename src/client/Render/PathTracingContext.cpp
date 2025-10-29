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
                              const std::shared_ptr<TLAS>& tlas, const std::vector<std::shared_ptr<BLAS>>& blasList)
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

    m_accumTexture = std::make_shared<Texture>();
    m_accumTexture->InitEmpty(device, Config::GetSystem().RTVFormat, Config::GetSystem().RtvWidth, Config::GetSystem().RtvHeight, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    m_accumTexture->Transition(cmdList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    m_accumClearBuffer = std::make_shared<Texture>();
    m_accumClearBuffer->InitEmpty(device, Config::GetSystem().RTVFormat, Config::GetSystem().RtvWidth, Config::GetSystem().RtvHeight);
    m_accumClearBuffer->Transition(cmdList, D3D12_RESOURCE_STATE_COPY_SOURCE);

    D3D12_STATIC_SAMPLER_DESC samplers[1];
    samplers[0] = {};
    samplers[0].Filter = D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
    samplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    samplers[0].ShaderRegister = 0;

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
    material->AddUAV(device, heap, m_accumTexture);
}

void PathTracingContext::Render(ID3D12GraphicsCommandList* cmdList, ID3D12RootSignature* rootSig,
                                ID3D12PipelineState* pso, const Camera* camera, const Material* material,
                                const XMMATRIX& projMatrix, const PtConfig& config)
{
    GPU_SCOPE(cmdList, L"Path Tracing");

    if (!config.RngPaused)
        m_curRngState = m_rngDist(m_rng);

    if (m_numFrames == 0)
    {
        m_accumTexture->Transition(cmdList, D3D12_RESOURCE_STATE_COPY_DEST);

        D3D12_TEXTURE_COPY_LOCATION dstLocation;
        dstLocation.pResource = m_accumTexture->GetD12Resource()->GetResource();
        dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        dstLocation.SubresourceIndex = 0;

        D3D12_TEXTURE_COPY_LOCATION srcLocation;
        srcLocation.pResource = m_accumClearBuffer->GetD12Resource()->GetResource();
        srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        srcLocation.SubresourceIndex = 0;

        D3D12_BOX srcBox;
        srcBox.front = 0;
        srcBox.back = 1;
        srcBox.left = 0;
        srcBox.right = Config::GetSystem().RtvWidth;
        srcBox.top = 0;
        srcBox.bottom = Config::GetSystem().RtvHeight;
        cmdList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, &srcBox);

        m_accumTexture->Transition(cmdList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    }

    {
        cmdList->SetGraphicsRootSignature(rootSig);
        cmdList->SetPipelineState(pso);

        CbvPathTracing cbv;
        cbv.CameraPositionWorld = camera->GetPosition();
        cbv.InvP = XMMatrixInverse(nullptr, projMatrix);
        cbv.InvV = XMMatrixInverse(nullptr, camera->GetViewMatrix());
        cbv.NumBounces = 2;
        cbv.Seed = m_curRngState;
        cbv.SPP = config.SPP;
        cbv.NumFrames = m_numFrames;
        cbv.AccumulationEnabled = config.AccumulationEnabled ? 1 : 0;
        cbv.WindowAppGuiWidth = Config::GetSystem().WindowAppGuiWidth;

        material->UpdateCBV(0, &cbv);
        material->SetDescriptorTables(cmdList);

        cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        cmdList->IASetVertexBuffers(0, 1, &m_fullScreenTriangle.GetVertexBufferView());
        cmdList->IASetIndexBuffer(&m_fullScreenTriangle.GetIndexBufferView());
        cmdList->DrawIndexedInstanced(static_cast<UINT>(m_fullScreenTriangle.GetIndexCount()), 1, 0, 0, 0);
    }

    m_numFrames++;
}

void PathTracingContext::Reset()
{
    m_numFrames = 0;
}
