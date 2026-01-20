//
// Created by fiona on 14/01/2026.
//

#include "System/pch.h"
#include "Render/Skybox.h"
#include "CBV.h"
#include "Debug/GPUEventScoped.h"
#include "HWI/Heap.h"

void Skybox::Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, Heap* heap, D12Resource* cubemap)
{
    m_pHeap = heap;

    D3D12_STATIC_SAMPLER_DESC samplers[1];
    samplers[0] = {};
    samplers[0].Filter = D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
    samplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
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

    // Init Generate Irradiance
    {
        m_rootSigGenIrr.SmartInit(device, 0, 1, 1, false, samplers, _countof(samplers));
        m_shaderGenIrr.InitCs(L"GenIrradianceIblCS.hlsl", device, m_rootSigGenIrr.Get());
    }
}

void Skybox::RenderForward(const D3D* d3d, ID3D12GraphicsCommandList* cmdList, const XMMATRIX& vMatrix, const XMMATRIX& pMatrix) const
{
    CbvMatrices matrices = {};
    matrices.V = vMatrix;
    matrices.P = pMatrix;

    cmdList->SetGraphicsRootSignature(m_rootSig.Get());
    cmdList->SetPipelineState(m_shaderForward.GetPSO());

    m_matForwardRender.UpdateCBV(0, &matrices);

    m_matForwardRender.TransitionSrvsToPS(cmdList);
    m_matForwardRender.SetDescriptorTables(cmdList);

    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmdList->IASetVertexBuffers(0, 1, &m_cube.GetVertexBufferView());
    cmdList->IASetIndexBuffer(&m_cube.GetIndexBufferView());
    cmdList->DrawIndexedInstanced(static_cast<UINT>(m_cube.GetIndexCount()), 1, 0, 0, 0);
}

void Skybox::UpdateCubemap(ID3D12Device* device, D12Resource* cubemap)
{
    // Forward Render Material
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
        srvDesc.Format = cubemap->GetDesc().Format;
        srvDesc.TextureCube.MipLevels = 1;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        m_matForwardRender.Init(m_pHeap, false);
        m_matForwardRender.AddCBV(device, m_pHeap, sizeof(CbvMatrices));
        m_matForwardRender.SetTex(device, 0, m_pHeap, cubemap);
    }

    // Generate Irradiance Material
    {
        m_texIrradianceIBL.InitEmpty(device, DXGI_FORMAT_R8G8B8A8_UNORM, cubemap->GetDesc().Width, cubemap->GetDesc().Height, 6, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
        uavDesc.Format = m_texIrradianceIBL.GetFormat();
        uavDesc.Texture2DArray.ArraySize = 6;
        uavDesc.Texture2DArray.MipSlice = 0;
        uavDesc.Texture2DArray.FirstArraySlice = 0;

        m_matGenIrr.Init(m_pHeap);
        m_matGenIrr.AddUAV(device, m_pHeap, m_texIrradianceIBL.GetD12Resource()->GetResource(), uavDesc);
        m_matGenIrr.SetTex(device, 0, m_pHeap, cubemap);
    }
}

void Skybox::GenerateIrradianceMap(ID3D12GraphicsCommandList* cmdList, const Heap* heap)
{
    GPU_SCOPE(cmdList, "Generate Irradiance Map");

    cmdList->SetComputeRootSignature(m_rootSigGenIrr.Get());
    cmdList->SetPipelineState(m_shaderGenIrr.GetPSO());

    m_texIrradianceIBL.Transition(cmdList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    heap->SetHeap(cmdList);
    m_matGenIrr.SetDescriptorTables(cmdList, true);
    m_matGenIrr.TransitionSrvsToPS(cmdList);

    const uint32_t groupWidth = (m_texIrradianceIBL.GetDesc().Width + 7) / 8;
    const uint32_t groupHeight = (m_texIrradianceIBL.GetDesc().Height + 7) / 8;

    cmdList->Dispatch(groupWidth, groupHeight, 1);
}
