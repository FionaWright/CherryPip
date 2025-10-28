//
// Created by fionaw on 28/09/2025.
//

#ifndef PT_MODEL_H
#define PT_MODEL_H
#include <d3d12.h>
#include <DirectXMath.h>
#include <memory>
#include <string>
#include <wrl/client.h>

#include "D12Resource.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

class Model
{
public:
    void Init(ID3D12Device* device, const size_t vertexCount, const size_t indexCount, const size_t vertexInputSize, const float boundingRadius, const XMFLOAT3 centroid);
    void SetBuffers(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const void* vBufferData, const void* iBufferData) const;

    void InitFullScreenTriangle(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);

    std::shared_ptr<D12Resource> GetVertexBuffer() const { return m_vertexBuffer; }
    std::shared_ptr<D12Resource> GetIndexBuffer() const { return m_indexBuffer; }
    const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() const { return m_vertexBufferView; }
    const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() const { return m_indexBufferView; }
    size_t GetVertexCount() const { return m_vertexCount; }
    size_t GetIndexCount() const { return m_indexCount; }
    size_t GetVertexInputSize() const { return m_vertexInputSize; }

private:
    std::shared_ptr<D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};

    std::shared_ptr<D12Resource> m_indexBuffer;
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView = {};

    size_t m_vertexCount = 0, m_indexCount = 0, m_vertexInputSize = 0;

    float m_boundingSphereRadius = 0.0f;
    XMFLOAT3 m_centroid = {};
    bool m_loadedData = false;
    std::string m_filepath = "";
};


#endif //PT_MODEL_H