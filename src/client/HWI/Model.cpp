//
// Created by fionaw on 28/09/2025.
//

#include "HWI/Model.h"

#include "Helper.h"

/*
bool Model::Init(ID3D12GraphicsCommandList2* cmdList, std::wstring filepath)
{
    std::vector<VertexInputData> vertexBuffer;
    std::vector<int32_t> indexBuffer;
    float radius;
    XMFLOAT3 centroid;

    m_filepath = wstringToString(filepath);

    bool r = ModelLoaderObj::LoadModel(filepath, vertexBuffer, indexBuffer, radius, centroid);
    if (!r)
        return false;

    Init(vertexBuffer.size(), indexBuffer.size(), sizeof(VertexInputData), radius, centroid);
    SetBuffers(cmdList, vertexBuffer.data(), indexBuffer.data());

    return true;
}

bool Model::Init(ID3D12GraphicsCommandList2* cmdList, std::string filepath)
{
    std::wstring wFilePath(filepath.begin(), filepath.end());
    return Init(cmdList, wFilePath);
}

void Model::Init(size_t vertexCount, size_t indexCount, size_t vertexInputSize, float boundingRadius, XMFLOAT3 centroid)
{
    m_boundingSphereRadius = boundingRadius;
    m_centroid = centroid;

    m_vertexCount = vertexCount;
    m_indexCount = indexCount;
    m_vertexInputSize = vertexInputSize;

    ComPtr<ID3D12Resource> intermediateVertexBuffer;
    ResourceManager::CreateCommittedResourceAsCommon(m_VertexBuffer, m_vertexCount, m_vertexInputSize);

    m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
    m_VertexBufferView.SizeInBytes = static_cast<UINT>(m_vertexCount * m_vertexInputSize);
    m_VertexBufferView.StrideInBytes = static_cast<UINT>(m_vertexInputSize);

    ComPtr<ID3D12Resource> intermediateIndexBuffer;
    ResourceManager::CreateCommittedResourceAsCommon(m_IndexBuffer, m_indexCount, sizeof(int32_t));

    m_IndexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
    m_IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
    m_IndexBufferView.SizeInBytes = static_cast<UINT>(m_indexCount * sizeof(int32_t));
}

void Model::SetBuffers(ID3D12GraphicsCommandList2* cmdList, const void* vBufferData, const void* iBufferData)
{
    ResourceManager::UploadCommittedResource(cmdList, m_VertexBuffer, &m_intermediateVertexBuffer, m_vertexCount, m_vertexInputSize, vBufferData);
    ResourceManager::UploadCommittedResource(cmdList, m_IndexBuffer, &m_intermediateIndexBuffer, m_indexCount, sizeof(int32_t), iBufferData);
}

*/