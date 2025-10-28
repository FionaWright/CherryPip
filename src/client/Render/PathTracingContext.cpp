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

    for (int i = 0; i < m_blasList.size(); i++)
    {
        PtInstanceData instanceData;
        instanceData.VertexBufferIdx = i;
        instanceData.IndexBufferIdx = i;
        instanceData.MaterialIdx = i;
        instanceData.M = m_blasList[i]->GetTransform().GetModelMatrix();
        m_instanceDataList.emplace_back(instanceData);
    }

    const UINT64 bufferSize = sizeof(PtInstanceData) * m_instanceDataList.size();
    m_instanceDataBuffer = std::make_shared<D12Resource>();
    m_instanceDataBuffer->Init(L"Path-Tracing Instance Data Buffer", device, bufferSize, D3D12_RESOURCE_STATE_COMMON);
    m_instanceDataBuffer->UploadData(device, cmdList, m_instanceDataList.data(), bufferSize);

    m_fullScreenTriangle.InitFullScreenTriangle(device, cmdList);
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
