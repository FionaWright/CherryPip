//
// Created by fionaw on 09/11/2025.
//

#include "../Headers/FurnaceTest.h"

#include "CBV.h"
#include "Helper.h"
#include "HWI/BLAS.h"
#include "HWI/Material.h"
#include "HWI/Shader.h"
#include "HWI/TLAS.h"
#include "Render/Object.h"
#include "Render/Scene.h"
#include "System/ModelLoaderGLTF.h"

void FurnaceTest::Init(const D3D* d3d, ID3D12GraphicsCommandList* cmdList, const std::vector<D3D12_INPUT_ELEMENT_DESC>& ildVec, ID3D12RootSignature* rootSig, Heap* heap)
{
    const D3D12_INPUT_LAYOUT_DESC ild = {ildVec.data(), static_cast<UINT>(ildVec.size())};
    m_shaderFurnaceClassic = std::make_shared<Shader>();
    m_shaderFurnaceClassic->InitVsPs(L"FullScreenTriangleVS.hlsl", L"Path-Tracing/FurnaceClassicPS.hlsl", ild,
                                     d3d->GetDevice(), rootSig);

    m_shaderFurnaceEmissive = std::make_shared<Shader>();
    m_shaderFurnaceEmissive->InitVsPs(L"FullScreenTriangleVS.hlsl", L"Path-Tracing/FurnaceEmissivePS.hlsl", ild,
                                      d3d->GetDevice(), rootSig);

    m_initialised = true;
}
