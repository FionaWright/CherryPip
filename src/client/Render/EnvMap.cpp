//
// Created by fionaw on 14/11/2025.
//

#include "Render/EnvMap.h"

#include "HWI/Heap.h"
#include "System/FileHelper.h"

void EnvMap::Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const std::wstring& filePath, float rotation, Heap* heap)
{
    D3D12_STATIC_SAMPLER_DESC samplers[1];
    samplers[0] = {};
    samplers[0].Filter = D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
    samplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    samplers[0].ShaderRegister = 0;

    m_rootSigPanoToEA.SmartInit(device, 1, 1, 1, false, samplers, _countof(samplers));
    m_shaderPanoToEA.InitCs(L"PanoToEaCS.hlsl", device, m_rootSigPanoToEA.Get());
    m_matPanoToEA.Init(heap);

    m_pano.Init(device, cmdList, FileHelper::GetAssetFullPath((L"Textures/" + filePath).c_str()), 1);
    m_ea.InitEmpty(device, DXGI_FORMAT_R8G8B8A8_UNORM, 4096, 4096, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

    m_pano.Transition(cmdList, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
    m_ea.Transition(cmdList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    struct CBV
    {
        uint32_t OutputWidth;
        uint32_t OutputHeight;
        uint32_t InputWidth;
        uint32_t InputHeight;

        float Rotation;
        XMFLOAT3 p;
    } cbv;
    cbv.OutputWidth = m_ea.GetDesc().Width;
    cbv.OutputHeight = m_ea.GetDesc().Height;
    cbv.InputWidth = m_pano.GetDesc().Width;
    cbv.InputHeight = m_pano.GetDesc().Height;
    cbv.Rotation = rotation;

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Format = m_pano.GetD12Resource()->GetDesc().Format;
    srvDesc.Texture2D.MipLevels = m_pano.GetD12Resource()->GetDesc().MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.PlaneSlice = 0;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    m_matPanoToEA.AddCBV(device, heap, sizeof(CBV));
    m_matPanoToEA.SetSRV(device, 0, heap, m_pano.GetD12Resource(), srvDesc);
    m_matPanoToEA.AddUAV(device, heap, m_ea.GetD12Resource()->GetResource(), m_ea.GetFormat());

    cmdList->SetComputeRootSignature(m_rootSigPanoToEA.Get());
    heap->SetHeap(cmdList);

    m_matPanoToEA.UpdateCBV(0, &cbv);
    m_matPanoToEA.SetDescriptorTables(cmdList, true);

    uint32_t groupSizeX = (m_pano.GetDesc().Width + 15) / 16;
    uint32_t groupSizeY = (m_pano.GetDesc().Height + 15) / 16;

    cmdList->SetPipelineState(m_shaderPanoToEA.GetPSO());
    cmdList->SetComputeRootSignature(m_rootSigPanoToEA.Get());
    cmdList->Dispatch(groupSizeX, groupSizeY, 1);

    m_ea.Transition(cmdList, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
}
