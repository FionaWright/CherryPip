//
// Created by fiona on 17/02/2026.
//

#ifndef CHERRYPIP_DEBUGLINE_H
#define CHERRYPIP_DEBUGLINE_H

#include "HWI/D3D.h"
#include "HWI/Material.h"
#include "HWI/Shader.h"

struct VsIn
{
    XMFLOAT3 Position;
    XMFLOAT3 Color;
};


class DebugLine
{
public:
    void Init(const D3D* d3d, Heap* heap, const std::shared_ptr<Shader>& shader);
    void Update(D3D* d3d, const XMFLOAT3* start = nullptr, const XMFLOAT3* end = nullptr, const XMFLOAT3* color = nullptr);
    void Render(ID3D12GraphicsCommandList* cmdList, const XMMATRIX& matrixVP);

private:
    XMFLOAT3 m_startPoint{}, m_endPoint{};
    XMFLOAT3 m_color{};

    std::shared_ptr<Shader> m_shader;
    Material m_material;

    ComPtr<ID3D12Resource> m_vertexBuffer;
    VsIn* m_mappedVertexData = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView{};
};


#endif //CHERRYPIP_DEBUGLINE_H