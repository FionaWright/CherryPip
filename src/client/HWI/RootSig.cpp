//
// Created by fionaw on 25/09/2025.
//

#include "HWI/RootSig.h"
#include "Helper.h"

void RootSig::Init(const CD3DX12_ROOT_PARAMETER1* params, const UINT paramCount, const D3D12_STATIC_SAMPLER_DESC* pSamplers, const UINT samplerCount, ID3D12Device* device)
{
    HRESULT hr;

    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    hr = device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData));
    if (FAILED(hr))
    {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    // Allow input layout and deny unnecessary access to certain pipeline stages.
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
    rootSignatureDescription.Init_1_1(paramCount, params, samplerCount, pSamplers, rootSignatureFlags);

    ComPtr<ID3DBlob> rootSignatureBlob;
    ComPtr<ID3DBlob> errorBlob;
    hr = D3DX12SerializeVersionedRootSignature(&rootSignatureDescription, featureData.HighestVersion, &rootSignatureBlob, &errorBlob);
    if (FAILED(hr))
    {
        if (errorBlob)
        {
            std::string errorMsg = static_cast<char*>(errorBlob->GetBufferPointer());
            std::wstring wideErrorMsg(errorMsg.begin(), errorMsg.end());
            wideErrorMsg = L"RootSig Error: " + wideErrorMsg;
            OutputDebugStringW(wideErrorMsg.c_str());
        }
        throw std::exception();
    }

    ComPtr<ID3D12RootSignature> rootSig;
    hr = device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSig));
    V(hr);

    m_rootSignature = rootSig;
}