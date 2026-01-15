//
// Created by fionaw on 14/11/2025.
//

#include "Render/EnvMap.h"

#include "HWI/Heap.h"
#include "System/FileHelper.h"

struct CBV_PanoToEA
{
    uint32_t OutputWidth;
    uint32_t OutputHeight;
    uint32_t InputWidth;
    uint32_t InputHeight;

    float Rotation;
    XMFLOAT3 p;
};

struct CBV_PanoToCM
{
    uint32_t OutputWidth;
    uint32_t InputWidth;
    uint32_t InputHeight;
    float Rotation;
};

void EnvMap::Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const std::wstring& filePath, const float rotation, Heap* heap)
{
    m_rotation = rotation;

    if (!m_resourcesInitialized)
        initResources(device);

    if (m_currentPanoFilepath != filePath)
    {
        m_pano.Init(device, cmdList, FileHelper::GetAssetFullPath((L"Textures/" + filePath).c_str()), 1);
        m_currentPanoFilepath = filePath;
    }

    m_pano.Transition(cmdList, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
    m_ea.Transition(cmdList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    CBV_PanoToEA cbv = {};
    cbv.OutputWidth = m_ea.GetDesc().Width;
    cbv.OutputHeight = m_ea.GetDesc().Height;
    cbv.InputWidth = m_pano.GetDesc().Width;
    cbv.InputHeight = m_pano.GetDesc().Height;
    cbv.Rotation = rotation / 360.0f;

    m_matPanoToEA.Init(heap);
    m_matPanoToEA.AddCBV(device, heap, sizeof(CBV_PanoToEA));
    m_matPanoToEA.SetTex(device, 0, heap, m_pano.GetD12Resource());
    m_matPanoToEA.AddUAV(device, heap, m_ea.GetD12Resource()->GetResource(), m_ea.GetFormat());

    cmdList->SetComputeRootSignature(m_rootSigPanoToEA.Get());
    heap->SetHeap(cmdList);

    m_matPanoToEA.UpdateCBV(0, &cbv);
    m_matPanoToEA.SetDescriptorTables(cmdList, true);

    const uint32_t groupSizeX = (m_ea.GetDesc().Width + 15) / 16;
    const uint32_t groupSizeY = (m_ea.GetDesc().Height + 15) / 16;

    cmdList->SetPipelineState(m_shaderPanoToEA.GetPSO());
    cmdList->Dispatch(groupSizeX, groupSizeY, 1);

    m_ea.Transition(cmdList, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
}

void EnvMap::InitCubemap(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, Heap* heap)
{
    assert(m_pano.IsInitialized());

    if (!m_cubemap.IsInitialized())
        m_cubemap.InitEmpty(device, DXGI_FORMAT_R8G8B8A8_UNORM, 1024, 1024, 6, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

    m_pano.Transition(cmdList, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
    m_cubemap.Transition(cmdList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    CBV_PanoToCM cbv = {};
    cbv.OutputWidth = m_cubemap.GetDesc().Width;
    cbv.InputWidth = m_pano.GetDesc().Width;
    cbv.InputHeight = m_pano.GetDesc().Height;
    cbv.Rotation = m_rotation / 360.0f;

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
    uavDesc.Format = m_cubemap.GetFormat();
    uavDesc.Texture2DArray.ArraySize = 6;
    uavDesc.Texture2DArray.MipSlice = 0;
    uavDesc.Texture2DArray.FirstArraySlice = 0;

    m_matPanoToCM.Init(heap);
    m_matPanoToCM.AddCBV(device, heap, sizeof(CBV));
    m_matPanoToCM.SetTex(device, 0, heap, m_pano.GetD12Resource());
    m_matPanoToCM.AddUAV(device, heap, m_cubemap.GetD12Resource()->GetResource(), uavDesc);

    cmdList->SetComputeRootSignature(m_rootSigPanoToCM.Get());
    heap->SetHeap(cmdList);

    m_matPanoToCM.UpdateCBV(0, &cbv);
    m_matPanoToCM.SetDescriptorTables(cmdList, true);

    const uint32_t groupSizeXY = (m_cubemap.GetDesc().Width + 15) / 16;

    cmdList->SetPipelineState(m_shaderPanoToCM.GetPSO());
    cmdList->Dispatch(groupSizeXY, groupSizeXY, 6);

    m_cubemap.Transition(cmdList, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
}

void EnvMap::initResources(ID3D12Device* device)
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

    m_ea.InitEmpty(device, DXGI_FORMAT_R8G8B8A8_UNORM, 4096, 4096, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

    m_rootSigPanoToCM.SmartInit(device, 1, 1, 1, false, samplers, _countof(samplers));
    m_shaderPanoToCM.InitCs(L"PanoToCubemapCS.hlsl", device, m_rootSigPanoToCM.Get());

    m_resourcesInitialized = true;
}