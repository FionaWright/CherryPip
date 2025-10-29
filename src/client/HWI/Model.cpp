//
// Created by fionaw on 28/09/2025.
//

#include "HWI/Model.h"

#include "Helper.h"

void Model::Init(ID3D12Device* device, const size_t vertexCount, const size_t indexCount, const size_t vertexInputSize, const float boundingRadius, const XMFLOAT3 centroid)
{
    m_boundingSphereRadius = boundingRadius;
    m_centroid = centroid;

    m_vertexCount = vertexCount;
    m_indexCount = indexCount;
    m_vertexInputSize = vertexInputSize;

    const size_t vBufferSize = m_vertexCount * m_vertexInputSize;
    m_vertexBuffer = std::make_shared<D12Resource>();
    m_vertexBuffer->InitBuffer(L"Vertex Buffer", device, vBufferSize, D3D12_RESOURCE_STATE_COMMON);

    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetResource()->GetGPUVirtualAddress();
    m_vertexBufferView.SizeInBytes = static_cast<UINT>(vBufferSize);
    m_vertexBufferView.StrideInBytes = static_cast<UINT>(m_vertexInputSize);

    const size_t iBufferSize = m_indexCount * sizeof(int32_t);
    m_indexBuffer = std::make_shared<D12Resource>();
    m_indexBuffer->InitBuffer(L"Index Buffer", device, iBufferSize, D3D12_RESOURCE_STATE_COMMON);

    m_indexBufferView.BufferLocation = m_indexBuffer->GetResource()->GetGPUVirtualAddress();
    m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
    m_indexBufferView.SizeInBytes = static_cast<UINT>(iBufferSize);
}

void Model::SetBuffers(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const void* vBufferData, const void* iBufferData) const
{
    const size_t vBufferSize = m_vertexCount * m_vertexInputSize;
    m_vertexBuffer->Transition(cmdList, D3D12_RESOURCE_STATE_COPY_DEST);
    m_vertexBuffer->UploadData(device, cmdList, vBufferData, vBufferSize);

    const size_t iBufferSize = m_indexCount * sizeof(int32_t);
    m_indexBuffer->Transition(cmdList, D3D12_RESOURCE_STATE_COPY_DEST);
    m_indexBuffer->UploadData(device, cmdList, iBufferData, iBufferSize);
}

void Model::InitFullScreenTriangle(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
    struct FullscreenVertex
    {
        XMFLOAT3 position;
        XMFLOAT2 uv;
    };

    static constexpr FullscreenVertex fullscreenVertices[] =
    {
        { XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },  // bottom-left
        { XMFLOAT3(-1.0f,  3.0f, 0.0f), XMFLOAT2(0.0f, -1.0f) }, // top-left (goes beyond screen)
        { XMFLOAT3( 3.0f, -1.0f, 0.0f), XMFLOAT2(2.0f, 1.0f) },  // bottom-right (goes beyond screen)
    };

    static constexpr uint32_t fullscreenIndices[] = { 0, 1, 2 };

    Init(device, 3, 3, sizeof(FullscreenVertex), 0, {});
    SetBuffers(device, cmdList, fullscreenVertices, fullscreenIndices);
}
