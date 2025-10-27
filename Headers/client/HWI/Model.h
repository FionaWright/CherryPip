//
// Created by fionaw on 28/09/2025.
//

#ifndef PT_MODEL_H
#define PT_MODEL_H
#include <d3d12.h>
#include <DirectXMath.h>
#include <string>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;
using namespace DirectX;

class Model
{
public:
    void Init(ID3D12Device* device, const size_t vertexCount, const size_t indexCount, const size_t vertexInputSize, const float boundingRadius, const XMFLOAT3 centroid);
    void SetBuffers(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const void* vBufferData, const void* iBufferData);

    ID3D12Resource* GetVertexBuffer() const { return m_vertexBuffer.Get(); }
    ID3D12Resource* GetIndexBuffer() const { return m_indexBuffer.Get(); }
    const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() const { return m_vertexBufferView; }
    const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() const { return m_indexBufferView; }
    size_t GetVertexCount() const { return m_vertexCount; }
    size_t GetIndexCount() const { return m_indexCount; }
    size_t GetVertexInputSize() const { return m_vertexInputSize; }

private:
    ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};

    ComPtr<ID3D12Resource> m_indexBuffer;
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView = {};

    ComPtr<ID3D12Resource> m_intermediateVertexBuffer;
    ComPtr<ID3D12Resource> m_intermediateIndexBuffer;

    size_t m_vertexCount = 0, m_indexCount = 0, m_vertexInputSize = 0;

    float m_boundingSphereRadius = 0.0f;
    XMFLOAT3 m_centroid = {};
    bool m_loadedData = false;
    std::string m_filepath = "";
};


#endif //PT_MODEL_H