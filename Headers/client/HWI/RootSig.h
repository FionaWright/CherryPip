//
// Created by fionaw on 25/09/2025.
//

#ifndef PT_ROOTSIG_H
#define PT_ROOTSIG_H

#include <windows.h>
#include <d3dx12.h>

using Microsoft::WRL::ComPtr;

class RootSig
{
public:
    void Init(const CD3DX12_ROOT_PARAMETER1* params, UINT paramCount, const D3D12_STATIC_SAMPLER_DESC* pSamplers, UINT samplerCount, ID3D12Device* device);

    ID3D12RootSignature* Get() const { return m_rootSignature.Get(); }

private:
    ComPtr<ID3D12RootSignature> m_rootSignature;
};


#endif //PT_ROOTSIG_H