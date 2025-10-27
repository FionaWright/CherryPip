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

    const size_t bufferSize = m_vertexCount * m_vertexInputSize;
    const auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    const auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, D3D12_RESOURCE_FLAG_NONE);
    V(device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_vertexBuffer)));
    V(m_vertexBuffer->SetName(L"Vertex Buffer"));

    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.SizeInBytes = static_cast<UINT>(bufferSize);
    m_vertexBufferView.StrideInBytes = static_cast<UINT>(m_vertexInputSize);

    const size_t bufferSizeIdx = m_indexCount * sizeof(int32_t);
    const auto heapPropertiesIdx = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    const auto resourceDescIdx = CD3DX12_RESOURCE_DESC::Buffer(bufferSizeIdx, D3D12_RESOURCE_FLAG_NONE);
    V(device->CreateCommittedResource(&heapPropertiesIdx, D3D12_HEAP_FLAG_NONE, &resourceDescIdx, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_indexBuffer)));
    V(m_indexBuffer->SetName(L"Index Buffer"));

    m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
    m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
    m_indexBufferView.SizeInBytes = static_cast<UINT>(bufferSizeIdx);
}

void Model::SetBuffers(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const void* vBufferData, const void* iBufferData)
{
    constexpr UINT64 offset = 0;
    constexpr UINT startIndex = 0;
    constexpr UINT resourceCount = 1;

    const size_t bufferSize = m_vertexCount * m_vertexInputSize;
    const auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    const auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
    V(device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_intermediateVertexBuffer)));
    D3D12_SUBRESOURCE_DATA subresourceData = {};
    subresourceData.pData = vBufferData;
    subresourceData.RowPitch = static_cast<LONG_PTR>(bufferSize);
    subresourceData.SlicePitch = subresourceData.RowPitch;
    UpdateSubresources(cmdList, m_vertexBuffer.Get(), m_intermediateVertexBuffer.Get(), offset, startIndex, resourceCount, &subresourceData);
    V(m_intermediateVertexBuffer->SetName(L"Intermediate Vertex Buffer"));

    const size_t bufferSizeIdx = m_indexCount * sizeof(int32_t);
    const auto heapPropertiesIdx = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    const auto resourceDescIdx = CD3DX12_RESOURCE_DESC::Buffer(bufferSizeIdx);
    V(device->CreateCommittedResource(&heapPropertiesIdx, D3D12_HEAP_FLAG_NONE, &resourceDescIdx, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_intermediateIndexBuffer)));
    subresourceData = {};
    subresourceData.pData = iBufferData;
    subresourceData.RowPitch = static_cast<LONG_PTR>(bufferSizeIdx);
    subresourceData.SlicePitch = subresourceData.RowPitch;
    UpdateSubresources(cmdList, m_indexBuffer.Get(), m_intermediateIndexBuffer.Get(), offset, startIndex, resourceCount, &subresourceData);
    V(m_intermediateIndexBuffer->SetName(L"Intermediate Index Buffer"));
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
