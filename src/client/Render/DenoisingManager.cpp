//
// Created by fiona on 15/01/2026.
//

#include "Render/DenoisingManager.h"

#include "CBV.h"

void DenoisingManager::Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, Heap* heap, D12Resource* pp1, D12Resource* pp2, D12Resource* normalsDepth)
{
    m_fullScreenTriangle.InitFullScreenTriangle(device, cmdList);

    m_ildDesc =
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
    m_ild = {m_ildDesc.data(), static_cast<uint32_t>(m_ildDesc.size())};

    // TODO: Don't initialize unused filters
    initBox(device, heap, pp1);
    initGauss(device, heap, pp1, pp2);
    initATrous(device, heap, pp1, pp2, normalsDepth);
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

    m_rootSigBox.SmartInit(device, 1, 1, 0, false, samplers, _countof(samplers));

    m_shaderBox.InitVsPs(L"FullScreenTriangleVS.hlsl", L"Filters/BoxPS.hlsl", m_ild, device, m_rootSigBox.Get());

    m_matBox.Init(heap);
    m_matBox.AddCBV(device, heap, sizeof(CbvFilterBoxAndGauss));
    m_matBox.SetTex(device, 0, heap, tex);
}

void DenoisingManager::initGauss(ID3D12Device* device, Heap* heap, D12Resource* pp1, D12Resource* pp2)
{
    D3D12_STATIC_SAMPLER_DESC samplers[1];
    samplers[0] = {};
    samplers[0].Filter = D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
    samplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    samplers[0].ShaderRegister = 0;

    m_rootSigGauss.SmartInit(device, 1, 1, 0, false, samplers, _countof(samplers));

    m_shaderGaussH.InitVsPs(L"FullScreenTriangleVS.hlsl", L"Filters/GaussianHPS.hlsl", m_ild, device, m_rootSigGauss.Get());
    m_shaderGaussV.InitVsPs(L"FullScreenTriangleVS.hlsl", L"Filters/GaussianVPS.hlsl", m_ild, device, m_rootSigGauss.Get());

    m_matGaussH.Init(heap);
    m_matGaussH.AddCBV(device, heap, sizeof(CbvFilterBoxAndGauss));
    m_matGaussH.SetTex(device, 0, heap, pp1);

    m_matGaussV.Init(heap);
    m_matGaussV.AddCBV(device, heap, sizeof(CbvFilterBoxAndGauss));
    m_matGaussV.SetTex(device, 0, heap, pp2);
}

void DenoisingManager::initATrous(ID3D12Device* device, Heap* heap, D12Resource* pp1, D12Resource* pp2, D12Resource* normalsDepth)
{
    D3D12_STATIC_SAMPLER_DESC samplers[1];
    samplers[0] = {};
    samplers[0].Filter = D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
    samplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    samplers[0].ShaderRegister = 0;

    m_rootSigATrous.SmartInit(device, 1, 2, 0, false, samplers, _countof(samplers));

    m_shaderATrous.InitVsPs(L"FullScreenTriangleVS.hlsl", L"Filters/ATrousPS.hlsl", m_ild, device, m_rootSigATrous.Get());

    m_matATrous1to2.Init(heap);
    m_matATrous1to2.AddCBV(device, heap, sizeof(CbvFilterATrous));
    m_matATrous1to2.SetTex(device, 0, heap, pp1);
    m_matATrous1to2.SetTex(device, 1, heap, normalsDepth);

    m_matATrous2to1.Init(heap);
    m_matATrous2to1.AddCBV(device, heap, sizeof(CbvFilterATrous));
    m_matATrous2to1.SetTex(device, 0, heap, pp2);
    m_matATrous2to1.SetTex(device, 1, heap, normalsDepth);
}

TextureRTV* DenoisingManager::DenoiseBox(ID3D12GraphicsCommandList* cmdList, TextureRTV* pp1, TextureRTV* pp2, const uint32_t radius) const
{
    pp2->GetD12Resource()->Transition(cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);
    const auto handle = pp2->GetCpuHandle();
    cmdList->OMSetRenderTargets(1, &handle, FALSE, nullptr);

    cmdList->SetGraphicsRootSignature(m_rootSigBox.Get());
    cmdList->SetPipelineState(m_shaderBox.GetPSO());

    CbvFilterBoxAndGauss cbv = {};
    cbv.TexelSize.x = 1.0f / static_cast<float>(pp1->GetD12Resource()->GetDesc().Width);
    cbv.TexelSize.y = 1.0f / static_cast<float>(pp1->GetD12Resource()->GetDesc().Height);
    cbv.Radius = radius;
    m_matBox.UpdateCBV(0, &cbv);

    m_matBox.TransitionSrvsToPS(cmdList);
    m_matBox.SetDescriptorTables(cmdList);

    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmdList->IASetVertexBuffers(0, 1, &m_fullScreenTriangle.GetVertexBufferView());
    cmdList->IASetIndexBuffer(&m_fullScreenTriangle.GetIndexBufferView());
    cmdList->DrawIndexedInstanced(static_cast<UINT>(m_fullScreenTriangle.GetIndexCount()), 1, 0, 0, 0);

    return pp2;
}

TextureRTV* DenoisingManager::DenoiseGauss(ID3D12GraphicsCommandList* cmdList, TextureRTV* pp1, TextureRTV* pp2, const uint32_t radius) const
{
    cmdList->SetGraphicsRootSignature(m_rootSigGauss.Get());

    CbvFilterBoxAndGauss cbv = {};
    cbv.TexelSize.x = 1.0f / static_cast<float>(pp1->GetD12Resource()->GetDesc().Width);
    cbv.TexelSize.y = 1.0f / static_cast<float>(pp1->GetD12Resource()->GetDesc().Height);
    cbv.Radius = radius;
    m_matGaussH.UpdateCBV(0, &cbv);
    m_matGaussV.UpdateCBV(0, &cbv);

    // Horizontal Pass (PP1 to PP2)
    {
        pp2->GetD12Resource()->Transition(cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);
        const auto handle = pp2->GetCpuHandle();
        cmdList->OMSetRenderTargets(1, &handle, FALSE, nullptr);

        m_matGaussH.TransitionSrvsToPS(cmdList);
        m_matGaussH.SetDescriptorTables(cmdList);

        cmdList->SetPipelineState(m_shaderGaussH.GetPSO());

        cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        cmdList->IASetVertexBuffers(0, 1, &m_fullScreenTriangle.GetVertexBufferView());
        cmdList->IASetIndexBuffer(&m_fullScreenTriangle.GetIndexBufferView());
        cmdList->DrawIndexedInstanced(static_cast<UINT>(m_fullScreenTriangle.GetIndexCount()), 1, 0, 0, 0);
    }

    // Vertical Pass (PP2 to PP1)
    {
        pp1->GetD12Resource()->Transition(cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);
        const auto handle = pp1->GetCpuHandle();
        cmdList->OMSetRenderTargets(1, &handle, FALSE, nullptr);

        m_matGaussV.TransitionSrvsToPS(cmdList);
        m_matGaussV.SetDescriptorTables(cmdList);

        cmdList->SetPipelineState(m_shaderGaussV.GetPSO());

        cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        cmdList->IASetVertexBuffers(0, 1, &m_fullScreenTriangle.GetVertexBufferView());
        cmdList->IASetIndexBuffer(&m_fullScreenTriangle.GetIndexBufferView());
        cmdList->DrawIndexedInstanced(static_cast<UINT>(m_fullScreenTriangle.GetIndexCount()), 1, 0, 0, 0);
    }

    return pp1;
}

TextureRTV* DenoisingManager::DenoiseATrous(ID3D12GraphicsCommandList* cmdList, const XMMATRIX& vMatrix, const XMMATRIX& pMatrix, TextureRTV* pp1, TextureRTV* pp2, const uint32_t iterations, const float phiC, const float phiN, const float phiP) const
{
    cmdList->SetGraphicsRootSignature(m_rootSigATrous.Get());

    CbvFilterATrous cbv = {};
    cbv.TexelSize.x = 1.0f / static_cast<float>(pp1->GetD12Resource()->GetDesc().Width);
    cbv.TexelSize.y = 1.0f / static_cast<float>(pp1->GetD12Resource()->GetDesc().Height);
    cbv.phiC = phiC;
    cbv.phiN = phiN;
    cbv.phiP = phiP;
    cbv.InvVP = XMMatrixInverse(nullptr, vMatrix * pMatrix);

    m_matATrous1to2.TransitionSrvsToPS(cmdList);
    m_matATrous2to1.TransitionSrvsToPS(cmdList);

    for (int i = 0; i < iterations; i++)
    {
        const bool pp1to2 = i % 2 == 0;
        TextureRTV* input = pp1to2 ? pp1 : pp2;
        TextureRTV* output = pp1to2 ? pp2 : pp1;

        input->GetD12Resource()->Transition(cmdList, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
        output->GetD12Resource()->Transition(cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);

        const auto handle = output->GetCpuHandle();
        cmdList->OMSetRenderTargets(1, &handle, FALSE, nullptr);

        const Material* mat = pp1to2 ? &m_matATrous1to2 : &m_matATrous2to1;
        cbv.StepWidth = pow(2, i);
        mat->UpdateCBV(0, &cbv);
        mat->SetDescriptorTables(cmdList);

        cmdList->SetPipelineState(m_shaderATrous.GetPSO());

        cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        cmdList->IASetVertexBuffers(0, 1, &m_fullScreenTriangle.GetVertexBufferView());
        cmdList->IASetIndexBuffer(&m_fullScreenTriangle.GetIndexBufferView());
        cmdList->DrawIndexedInstanced(static_cast<UINT>(m_fullScreenTriangle.GetIndexCount()), 1, 0, 0, 0);
    }

    return iterations % 2 == 0 ? pp1 : pp2;
}