//
// Created by fiona on 25/09/2025.
//

#ifndef PT_SHADER_H
#define PT_SHADER_H

#include "D3D.h"

class Shader
{
public:
    void InitVsPs(LPCWSTR vs, LPCWSTR ps, D3D12_INPUT_LAYOUT_DESC ild, ID3D12Device* device, ID3D12RootSignature* rootSig);
    void InitCs(LPCWSTR cs, ID3D12Device* device, ID3D12RootSignature* rootSig);

    ID3D12PipelineState* GetPSO() const { return m_pso.Get(); }

private:
    ComPtr<ID3D12PipelineState> m_pso;
};


#endif //PT_SHADER_H