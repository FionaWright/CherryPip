//
// Created by fionaw on 25/09/2025.
//

#include "HWI/RootSig.h"
#include "Helper.h"
#include "fastgltf/types.hpp"

void RootSig::Init(ID3D12Device* device, const CD3DX12_ROOT_PARAMETER1* params, const UINT paramCount, const D3D12_STATIC_SAMPLER_DESC* pSamplers, const UINT samplerCount)
{
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    HRESULT hr = device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData));
    if (FAILED(hr))
    {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    // Allow input layout and deny unnecessary access to certain pipeline stages.
    constexpr D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
    rootSignatureDescription.Init_1_1(paramCount, params, samplerCount, pSamplers, rootSignatureFlags);

    ComPtr<ID3DBlob> rootSignatureBlob;
    ComPtr<ID3DBlob> errorBlob;
    V(D3DX12SerializeVersionedRootSignature(&rootSignatureDescription, featureData.HighestVersion, &rootSignatureBlob, &errorBlob));

    ComPtr<ID3D12RootSignature> rootSig;
    V(device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSig)));
    V(rootSig->SetName(L"Root Signature"));

    m_rootSignature = rootSig;
}

void RootSig::SmartInit(ID3D12Device* device, const UINT numCBV, const UINT numSRV, const D3D12_STATIC_SAMPLER_DESC* samplers, const UINT samplerCount)
{
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    HRESULT hr = device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData));
    if (FAILED(hr))
    {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    // Assume CBV, SRV order
    std::vector<CD3DX12_ROOT_PARAMETER1> params;
    if (numCBV > 0)
    {
        CD3DX12_DESCRIPTOR_RANGE1 range;
        range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, numCBV, 0);

        CD3DX12_ROOT_PARAMETER1 param;
        param.InitAsDescriptorTable(1, &range, D3D12_SHADER_VISIBILITY_PIXEL);
        params.emplace_back(param);
    }
    if (numSRV > 0)
    {
        CD3DX12_DESCRIPTOR_RANGE1 range;
        range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, numSRV, 0);

        CD3DX12_ROOT_PARAMETER1 param;
        param.InitAsDescriptorTable(1, &range, D3D12_SHADER_VISIBILITY_PIXEL);
        params.emplace_back(param);
    }

    // Allow input layout and deny unnecessary access to certain pipeline stages.
    constexpr D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
    rootSignatureDescription.Init_1_1(params.size(), params.data(), samplerCount, samplers, rootSignatureFlags);

    ComPtr<ID3DBlob> rootSignatureBlob;
    ComPtr<ID3DBlob> errorBlob;
    V(D3DX12SerializeVersionedRootSignature(&rootSignatureDescription, featureData.HighestVersion, &rootSignatureBlob, &errorBlob));

    ComPtr<ID3D12RootSignature> rootSig;
    V(device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSig)));
    V(rootSig->SetName(L"Root Signature"));

    m_rootSignature = rootSig;
}