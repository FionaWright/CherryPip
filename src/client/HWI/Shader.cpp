//
// Created by fiona on 25/09/2025.
//

#include "HWI/Shader.h"

#include "Helper.h"
#include "System/FileHelper.h"

void Shader::Init(LPCWSTR vs, LPCWSTR ps, D3D12_INPUT_LAYOUT_DESC ild, ID3D12Device* device, ID3D12RootSignature* rootSig)
{
#if defined(_DEBUG)
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    UINT compileFlags = 0;
#endif

    ComPtr<IDxcBlob> vertexShader = CompileShaderDXC(FileHelper::GetAssetShaderFullPath(vs).c_str(), L"VSMain", L"vs_6_5", compileFlags);
    ComPtr<IDxcBlob> pixelShader = CompileShaderDXC(FileHelper::GetAssetShaderFullPath(ps).c_str(), L"PSMain", L"ps_6_5", compileFlags);
    
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = ild;
    psoDesc.pRootSignature = rootSig;
    psoDesc.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
    psoDesc.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;
    V(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pso)));
}
