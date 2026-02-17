//
// Created by fiona on 17/02/2026.
//

#include "System/pch.h"

#include "Debug/DebugLine.h"

#include "CBV.h"
#include "Helper.h"

void DebugLine::Init(const D3D* d3d, Heap* heap, const std::shared_ptr<Shader>& shader)
{
    m_shader = shader;

    m_material.Init(heap);
    m_material.AddCBV(d3d->GetDevice(), heap, sizeof(CbvMatrixVP), "CBV Line Matrix");

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Width = sizeof(VsIn) * 2; // Size for two vertices
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    V(d3d->GetDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_vertexBuffer)));

    m_vertexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedVertexData));

    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.StrideInBytes = sizeof(VsIn);
    m_vertexBufferView.SizeInBytes = sizeof(VsIn) * 2;
}

void DebugLine::Update(D3D* d3d, const XMFLOAT3* start, const XMFLOAT3* end, const XMFLOAT3* color)
{
    if (start)
        m_startPoint = *start;
    if (end)
        m_endPoint = *end;
    if (color)
        m_color = *color;

    // Copy data to GPU
    {
        d3d->Flush();

        const VsIn vertices[] = {
            { m_startPoint, m_color},
            { m_endPoint, m_color }
        };
        memcpy(m_mappedVertexData, vertices, sizeof(vertices));
    }
}

void DebugLine::Render(ID3D12GraphicsCommandList* cmdList, const XMMATRIX& matrixVP)
{
    cmdList->SetPipelineState(m_shader->GetPSO());
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

    CbvMatrixVP cbv;
    XMStoreFloat4x4(&cbv.VP, matrixVP);
    m_material.UpdateCBV(0, &cbv);
    m_material.SetDescriptorTables(cmdList);

    cmdList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    cmdList->DrawInstanced(2, 1, 0, 0);
}
