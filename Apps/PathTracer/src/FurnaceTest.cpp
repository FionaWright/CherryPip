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
#include "System/ModelLoaderGLTF.h"

void FurnaceTest::Init(D3D* d3d, ID3D12GraphicsCommandList* cmdList, const std::vector<D3D12_INPUT_ELEMENT_DESC>& ildVec, ID3D12RootSignature* rootSig, Heap* heap)
{
    m_furnaceTestSphere = ModelLoaderGLTF::LoadModelsFromGLTF(d3d, cmdList, L"Sphere/Sphere.gltf").at(0);

    ComPtr<ID3D12Device5> device5;
    V(d3d->GetDevice()->QueryInterface(IID_PPV_ARGS(&device5)));
    ComPtr<ID3D12GraphicsCommandList4> cmdList4;
    V(cmdList->QueryInterface(IID_PPV_ARGS(&cmdList4)));

    std::vector<std::shared_ptr<BLAS>> blasList;
    std::vector<PtMaterialData> materialData;
    auto blas = std::make_shared<BLAS>();
    blas->Init(device5.Get(), cmdList4.Get(), m_furnaceTestSphere, {});
    blasList.emplace_back(blas);

    PtMaterialData ptMaterialData;
    ptMaterialData.BaseColorFactor = XMFLOAT3(1, 1, 1);
    ptMaterialData.EmissiveStrength = 0;
    materialData.emplace_back(ptMaterialData);

    const D3D12_INPUT_LAYOUT_DESC ild = {ildVec.data(), static_cast<UINT>(ildVec.size())};
    m_shaderFurnaceClassic = std::make_shared<Shader>();
    m_shaderFurnaceClassic->InitVsPs(L"FullScreenTriangleVS.hlsl", L"Path-Tracing/FurnaceClassicPS.hlsl", ild,
                                     d3d->GetDevice(), rootSig);

    m_shaderFurnaceEmissive = std::make_shared<Shader>();
    m_shaderFurnaceEmissive->InitVsPs(L"FullScreenTriangleVS.hlsl", L"Path-Tracing/FurnaceEmissivePS.hlsl", ild,
                                      d3d->GetDevice(), rootSig);

    auto tlas = std::make_shared<TLAS>();
    tlas->Init(device5.Get(), cmdList4.Get(), blasList);
    m_ptContextFurnaceTest.Init(d3d->GetDevice(), cmdList, tlas, blasList, materialData);

    m_materialFurnace = std::make_shared<Material>();
    m_materialFurnace->Init(heap);
    m_materialFurnace->AddCBV(d3d->GetDevice(), heap, sizeof(CbvPathTracing));
    m_ptContextFurnaceTest.FillMaterial(d3d->GetDevice(), m_materialFurnace.get(), heap);

    m_initialised = true;
}
