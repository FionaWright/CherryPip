//
// Created by fiona on 14/01/2026.
//

#include "Render/Skybox.h"

#include "CBV.h"

void Skybox::Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, Heap* heap, D12Resource* cubemap)
{
    m_pHeap = heap;

    D3D12_STATIC_SAMPLER_DESC samplers[1];
    samplers[0] = {};
    samplers[0].Filter = D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
    samplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    samplers[0].ShaderRegister = 0;

    m_rootSig.SmartInit(device, 1, 1, 0, false, samplers, _countof(samplers));

    D3D12_INPUT_ELEMENT_DESC rasterILD[] =
    {
        {
            "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
        }
    };
    m_shaderForward.InitVsPs(L"Raster/SkyboxVS.hlsl", L"Raster/SkyboxPS.hlsl", {rasterILD, _countof(rasterILD)}, device, m_rootSig.Get(), true);

    constexpr XMFLOAT3 vertexBuffer[8] = {
        {-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1},
        {-1, -1,  1}, {1, -1,  1}, {1, 1,  1}, {-1, 1,  1}
    };

    constexpr uint32_t indexBuffer[36] = {
        0, 1, 2,  // back face
        0, 2, 3,
        4, 6, 5,  // front face
        4, 7, 6,
        0, 3, 7,  // left face
        0, 7, 4,
        1, 6, 2,  // right face
        1, 5, 6,
        0, 5, 1,  // bottom face
        0, 4, 5,
        3, 6, 7,  // top face
        3, 2, 6
    };

    m_cube.Init(device, _countof(vertexBuffer), _countof(indexBuffer), sizeof(XMFLOAT3), 1000, XMFLOAT3(0,0,0));
    m_cube.SetBuffers(device, cmdList, vertexBuffer, indexBuffer);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.Format = cubemap->GetDesc().Format;
    srvDesc.TextureCube.MipLevels = 1;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    m_mat.Init(heap, false);
    m_mat.AddCBV(device, heap, sizeof(CbvMatrices));
    m_mat.SetSRV(device, 0, heap, cubemap, srvDesc);
}

void Skybox::RenderForward(const D3D* d3d, ID3D12GraphicsCommandList* cmdList, const XMMATRIX& vMatrix, const XMMATRIX& pMatrix) const
{
    CbvMatrices matrices = {};
    matrices.V = vMatrix;
    matrices.P = pMatrix;

    cmdList->SetGraphicsRootSignature(m_rootSig.Get());
    cmdList->SetPipelineState(m_shaderForward.GetPSO());

    m_mat.UpdateCBV(0, &matrices);

    m_mat.TransitionSrvsToPS(cmdList);
    m_mat.SetDescriptorTables(cmdList);

    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmdList->IASetVertexBuffers(0, 1, &m_cube.GetVertexBufferView());
    cmdList->IASetIndexBuffer(&m_cube.GetIndexBufferView());
    cmdList->DrawIndexedInstanced(static_cast<UINT>(m_cube.GetIndexCount()), 1, 0, 0, 0);
}