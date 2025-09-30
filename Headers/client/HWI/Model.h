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
    bool Init(ID3D12GraphicsCommandList2* cmdList, std::wstring filepath);
    bool Init(ID3D12GraphicsCommandList2* cmdList, std::string filepath);
    void Init(size_t vertexCount, size_t indexCount, size_t vertexInputSize, float boundingRadius, XMFLOAT3 centroid);
    void SetBuffers(ID3D12GraphicsCommandList2* cmdList, const void* vBufferData, const void* iBufferData);

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