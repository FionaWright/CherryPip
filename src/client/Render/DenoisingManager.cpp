//
// Created by fiona on 15/01/2026.
//

#include "Render/DenoisingManager.h"

#include "CBV.h"

void DenoisingManager::Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, Heap* heap, D12Resource* tex)
{
    m_fullScreenTriangle.InitFullScreenTriangle(device, cmdList);

    initBox(device, heap, tex);
}

void DenoisingManager::DenoiseBox(ID3D12GraphicsCommandList* cmdList, const D12Resource* tex, const uint32_t radius) const
{
    cmdList->SetGraphicsRootSignature(m_rootSigBox.Get());
    cmdList->SetPipelineState(m_shaderBox.GetPSO());

    CbvFilterBox cbv = {};
    cbv.TexelSize = XMFLOAT2(1.0f / static_cast<float>(tex->GetDesc().Width), 1.0f / static_cast<float>(tex->GetDesc().Height));
    cbv.Radius = radius;
    m_matBox.UpdateCBV(0, &cbv);

    m_matBox.TransitionSrvsToPS(cmdList);
    m_matBox.SetDescriptorTables(cmdList);

    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmdList->IASetVertexBuffers(0, 1, &m_fullScreenTriangle.GetVertexBufferView());
    cmdList->IASetIndexBuffer(&m_fullScreenTriangle.GetIndexBufferView());
    cmdList->DrawIndexedInstanced(static_cast<UINT>(m_fullScreenTriangle.GetIndexCount()), 1, 0, 0, 0);
}

void DenoisingManager::initBox(ID3D12Device* device, Heap* heap, D12Resource* tex)
{
    D3D12_STATIC_SAMPLER_DESC samplers[1];
    samplers[0] = {};
    samplers[0].Filter = D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
    samplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    samplers[0].ShaderRegister = 0;

    m_rootSigBox.SmartInit(device, 1, 1, 1, false, samplers, _countof(samplers));

    D3D12_INPUT_ELEMENT_DESC ild[] =
    {
        {
            "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
        },
        {
            "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
        }
    };

    m_shaderBox.InitVsPs(L"FullScreenTriangleVS.hlsl", L"Filters/BoxPS.hlsl", {ild, _countof(ild)}, device, m_rootSigBox.Get());

    m_matBox.Init(heap);
    m_matBox.AddCBV(device, heap, sizeof(CbvFilterBox));
    m_matBox.SetTex(device, 0, heap, tex);
}
