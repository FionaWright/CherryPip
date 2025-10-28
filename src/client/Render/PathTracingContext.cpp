//
// Created by fionaw on 28/10/2025.
//

#include "Render/PathTracingContext.h"

#include <d3dx12_core.h>

#include "CBV.h"
#include "Helper.h"
#include "Debug/GPUEventScoped.h"
#include "HWI/BLAS.h"
#include "HWI/Material.h"
#include "Render/Camera.h"

void PathTracingContext::Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const std::shared_ptr<TLAS>& tlas, const std::vector<std::shared_ptr<BLAS>>& blasList)
{
    m_tlas = tlas;
    m_blasList = blasList;

    UINT curVertexBufferOffset = 0;
    UINT curIndexBufferOffset = 0;
    for (int i = 0; i < m_blasList.size(); i++)
    {
        const Model* model = m_blasList[i]->GetModel();

        PtInstanceData instanceData;
        instanceData.VertexBufferOffset = curVertexBufferOffset;
        instanceData.IndexBufferOffset = curIndexBufferOffset;
        instanceData.MaterialIdx = i;
        instanceData.M = m_blasList[i]->GetTransform().GetModelMatrix();
        m_instanceDataList.emplace_back(instanceData);

        curVertexBufferOffset += model->GetVertexCount();
        curIndexBufferOffset += model->GetIndexCount() / 3;
    }

    m_vertexMegaBufferCount = curVertexBufferOffset;
    m_indexMegaBufferCount = curIndexBufferOffset;

    const UINT64 vMegaBufferSize = sizeof(Vertex) * m_vertexMegaBufferCount;
    m_vertexMegaBuffer = std::make_shared<D12Resource>();
    m_vertexMegaBuffer->Init(L"Path-Tracing Vertex Mega Buffer", device, vMegaBufferSize, D3D12_RESOURCE_STATE_COPY_DEST);

    const UINT64 iMegaBufferSize = sizeof(uint32_t) * 3 * m_indexMegaBufferCount;
    m_indexMegaBuffer = std::make_shared<D12Resource>();
    m_indexMegaBuffer->Init(L"Path-Tracing Index Mega Buffer", device, iMegaBufferSize, D3D12_RESOURCE_STATE_COPY_DEST);

    const UINT64 bufferSize = sizeof(PtInstanceData) * m_instanceDataList.size();
    m_instanceDataBuffer = std::make_shared<D12Resource>();
    m_instanceDataBuffer->Init(L"Path-Tracing Instance Data Buffer", device, bufferSize, D3D12_RESOURCE_STATE_COMMON);
    m_instanceDataBuffer->UploadData(device, cmdList, m_instanceDataList.data(), bufferSize);

    size_t vByteOffset = 0;
    size_t iByteOffset = 0;
    for (int i = 0; i < m_blasList.size(); i++)
    {
        const Model* model = m_blasList[i]->GetModel();
        auto vBuffer = model->GetVertexBuffer();
        auto iBuffer = model->GetIndexBuffer();

        vBuffer->Transition(cmdList, D3D12_RESOURCE_STATE_COPY_SOURCE);
        iBuffer->Transition(cmdList, D3D12_RESOURCE_STATE_COPY_SOURCE);

        const UINT vBufferSize = sizeof(Vertex) * model->GetVertexCount();
        cmdList->CopyBufferRegion(m_vertexMegaBuffer->GetResource(), vByteOffset, vBuffer->GetResource(), 0, vBufferSize);

        const UINT iBufferSize = sizeof(uint32_t) * model->GetIndexCount();
        cmdList->CopyBufferRegion(m_indexMegaBuffer->GetResource(), iByteOffset, iBuffer->GetResource(), 0, iBufferSize);

        vByteOffset += vBufferSize;
        iByteOffset += iBufferSize;
    }

    m_fullScreenTriangle.InitFullScreenTriangle(device, cmdList);
}

void PathTracingContext::FillMaterial(ID3D12Device* device, Material* material, Heap* heap) const
{
    material->AddTLAS(device, heap, m_tlas);
    material->AddBuffer(device, heap, m_instanceDataBuffer, m_instanceDataList.size(), sizeof(PtInstanceData));
    material->AddBuffer(device, heap, m_vertexMegaBuffer, m_vertexMegaBufferCount, sizeof(Vertex));
    material->AddBuffer(device, heap, m_indexMegaBuffer, m_indexMegaBufferCount, sizeof(uint32_t) * 3);
}

void PathTracingContext::Render(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, ID3D12RootSignature* rootSig, ID3D12PipelineState* pso, const Camera* camera, const Material* material, const XMMATRIX& projMatrix) const
{
    GPU_SCOPE(cmdList, L"Path Tracing");

    cmdList->SetGraphicsRootSignature(rootSig);
    cmdList->SetPipelineState(pso);

    CbvPathTracing cbv;
    cbv.CameraPositionWorld = camera->GetPosition();
    cbv.InvP = XMMatrixInverse(nullptr, projMatrix);
    cbv.InvV = XMMatrixInverse(nullptr, camera->GetViewMatrix());

    material->UpdateCBV(0, &cbv);
    material->SetDescriptorTables(cmdList);

    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmdList->IASetVertexBuffers(0, 1, &m_fullScreenTriangle.GetVertexBufferView());
    cmdList->IASetIndexBuffer(&m_fullScreenTriangle.GetIndexBufferView());
    cmdList->DrawIndexedInstanced(static_cast<UINT>(m_fullScreenTriangle.GetIndexCount()), 1, 0, 0, 0);
}


