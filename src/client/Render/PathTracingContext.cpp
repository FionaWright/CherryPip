//
// Created by fionaw on 28/10/2025.
//

#include "System/pch.h"
#include "Render/PathTracingContext.h"

#include "CBV.h"
#include "Helper.h"
#include "../../../Headers/client/Render/Scene.h"
#include "Debug/GPUEventScoped.h"
#include "HWI/BLAS.h"
#include "HWI/Material.h"
#include "HWI/TLAS.h"
#include "Render/Camera.h"
#include "Render/Object.h"
#include "System/Config.h"

void PathTracingContext::Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, Heap* heap)
{
    m_fullScreenTriangle.InitFullScreenTriangle(device, cmdList);

    m_accumTexture = std::make_shared<Texture>();
    m_accumTexture->InitEmpty(device, DXGI_FORMAT_R32G32B32A32_FLOAT, Config::GetSystem().RtvWidth, Config::GetSystem().RtvHeight, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    m_accumTexture->Transition(cmdList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    m_accumClearBuffer = std::make_shared<Texture>();
    m_accumClearBuffer->InitEmpty(device, m_accumTexture->GetFormat(), Config::GetSystem().RtvWidth, Config::GetSystem().RtvHeight);
    m_accumClearBuffer->Transition(cmdList, D3D12_RESOURCE_STATE_COPY_SOURCE);

    m_material = std::make_shared<Material>();
    m_material->Init(heap, true);
    m_material->AddCBV(device, heap, sizeof(CbvPathTracing), "CBV Path Tracing");
#ifdef _DEBUG
    m_material->AddCBV(device, heap, sizeof(CbvPathTracingDebug), "CBV Path Tracing Debug");
#endif
    m_material->AddUAV(device, heap, m_accumTexture);
}

void PathTracingContext::BuildScene(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const Scene* scene, Heap* heap, D12Resource* envMap)
{
    m_instanceDataList.clear();
    m_blasList.clear();
    m_tlas.reset();
    m_instanceDataBuffer.reset();
    m_vertexMegaBuffer.reset();
    m_indexMegaBuffer.reset();
    m_materialBuffer.reset();

    ComPtr<ID3D12Device5> device5;
    V(device->QueryInterface(IID_PPV_ARGS(&device5)));
    ComPtr<ID3D12GraphicsCommandList4> cmdList4;
    V(cmdList->QueryInterface(IID_PPV_ARGS(&cmdList4)));

    const auto& objects = scene->GetObjects();
    std::vector<PtMaterialData> materialData;
    for (int i = 0; i < objects.size(); i++)
    {
        const Object* object = objects[i].get();

        auto blas = std::make_shared<BLAS>();
        blas->Init(device5.Get(), cmdList4.Get(), object->GetModel(), *object->GetTransform());
        m_blasList.emplace_back(blas);

        const MaterialData* objectMaterialData = object->GetMaterial()->GetData();
        PtMaterialData ptMaterialData;
        ptMaterialData.BaseColorFactor = objectMaterialData->BaseColorFactor;
        ptMaterialData.EmissiveStrength = objectMaterialData->EmissiveStrength;
        ptMaterialData.Roughness = objectMaterialData->Roughness;
        ptMaterialData.Metalness = objectMaterialData->Metalness;
        ptMaterialData.DiffuseProbability = objectMaterialData->DiffuseProbability;
        ptMaterialData.TexIdxAlbedo = objectMaterialData->BindlessTexDiffuse;
        ptMaterialData.TexIdxNormal = objectMaterialData->BindlessTexNormal;
        ptMaterialData.TexIdxRoughMet = objectMaterialData->BindlessTexRoughMet;
        ptMaterialData.TexIdxEmissive = objectMaterialData->BindlessTexEmissive;
        ptMaterialData.IoR = objectMaterialData->IoR;
        ptMaterialData.Flags = objectMaterialData->Flags;
        ptMaterialData.EmissiveColor = objectMaterialData->EmissiveColor;
        materialData.emplace_back(ptMaterialData);
    }

    m_tlas = std::make_shared<TLAS>();
    m_tlas->Init(device5.Get(), cmdList4.Get(), m_blasList);

    UINT curVertexBufferOffset = 0;
    UINT curIndexBufferOffset = 0;
    for (int i = 0; i < m_blasList.size(); i++)
    {
        const Model* model = m_blasList[i]->GetModel();

        PtInstanceData instanceData;
        instanceData.VertexBufferOffset = curVertexBufferOffset;
        instanceData.IndexBufferOffset = curIndexBufferOffset;
        instanceData.MaterialIdx = i;

        XMMATRIX M = m_blasList[i]->GetTransform().GetModelMatrix();
        XMStoreFloat4x4(&instanceData.M, M);
        XMStoreFloat4x4(&instanceData.MTI, XMMatrixTranspose(XMMatrixInverse(nullptr, M)));
        m_instanceDataList.emplace_back(instanceData);

        curVertexBufferOffset += model->GetVertexCount();
        curIndexBufferOffset += model->GetIndexCount() / 3;
    }

    m_vertexMegaBufferCount = curVertexBufferOffset;
    m_indexMegaBufferCount = curIndexBufferOffset;

    const UINT64 vMegaBufferSize = sizeof(Vertex) * m_vertexMegaBufferCount;
    m_vertexMegaBuffer = std::make_shared<D12Resource>();
    m_vertexMegaBuffer->InitBuffer(L"Path-Tracing Vertex Mega Buffer", device, vMegaBufferSize);
    m_vertexMegaBuffer->Transition(cmdList, D3D12_RESOURCE_STATE_COPY_DEST);

    const UINT64 iMegaBufferSize = sizeof(uint32_t) * 3 * m_indexMegaBufferCount;
    m_indexMegaBuffer = std::make_shared<D12Resource>();
    m_indexMegaBuffer->InitBuffer(L"Path-Tracing Index Mega Buffer", device, iMegaBufferSize);
    m_indexMegaBuffer->Transition(cmdList, D3D12_RESOURCE_STATE_COPY_DEST);

    const UINT64 bufferSize = sizeof(PtInstanceData) * m_instanceDataList.size();
    m_instanceDataBuffer = std::make_shared<D12Resource>();
    m_instanceDataBuffer->InitBuffer(L"Path-Tracing Instance Data Buffer", device, bufferSize);
    m_instanceDataBuffer->UploadBuffer(device, cmdList, m_instanceDataList.data(), bufferSize);
    m_instanceDataBuffer->Transition(cmdList, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);

    const UINT64 mBufferSize = sizeof(PtMaterialData) * m_instanceDataList.size();
    m_materialBuffer = std::make_shared<D12Resource>();
    m_materialBuffer->InitBuffer(L"Path-Tracing Material Data Buffer", device, mBufferSize);
    m_materialBuffer->UploadBuffer(device, cmdList, materialData.data(), mBufferSize);
    m_materialBuffer->Transition(cmdList, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);

    size_t vByteOffset = 0;
    size_t iByteOffset = 0;
    for (int i = 0; i < m_blasList.size(); i++)
    {
        const Model* model = m_blasList[i]->GetModel();
        const auto vBuffer = model->GetVertexBuffer();
        const auto iBuffer = model->GetIndexBuffer();

        vBuffer->Transition(cmdList, D3D12_RESOURCE_STATE_COPY_SOURCE);
        iBuffer->Transition(cmdList, D3D12_RESOURCE_STATE_COPY_SOURCE);

        const UINT vBufferSize = sizeof(Vertex) * model->GetVertexCount();
        cmdList->CopyBufferRegion(m_vertexMegaBuffer->GetResource(), vByteOffset, vBuffer->GetResource(), 0,
                                  vBufferSize);

        const UINT iBufferSize = sizeof(uint32_t) * model->GetIndexCount();
        cmdList->CopyBufferRegion(m_indexMegaBuffer->GetResource(), iByteOffset, iBuffer->GetResource(), 0,
                                  iBufferSize);

        vBuffer->Transition(cmdList, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
        iBuffer->Transition(cmdList, D3D12_RESOURCE_STATE_INDEX_BUFFER);

        vByteOffset += vBufferSize;
        iByteOffset += iBufferSize;
    }
    m_vertexMegaBuffer->Transition(cmdList, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
    m_indexMegaBuffer->Transition(cmdList, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);

    m_material->SetTlas(device,0, heap, m_tlas);
    m_material->SetBuffer(device, 1, heap, m_instanceDataBuffer, m_instanceDataList.size(), sizeof(PtInstanceData));
    m_material->SetBuffer(device, 2, heap, m_vertexMegaBuffer, m_vertexMegaBufferCount, sizeof(Vertex));
    m_material->SetBuffer(device, 3, heap, m_indexMegaBuffer, m_indexMegaBufferCount, sizeof(uint32_t) * 3);
    m_material->SetBuffer(device, 4, heap, m_materialBuffer, m_instanceDataList.size(), sizeof(PtMaterialData));
    m_material->SetTex(device, 5, heap, envMap);
}

void PathTracingContext::Render(ID3D12GraphicsCommandList* cmdList, ID3D12RootSignature* rootSig,
                                ID3D12PipelineState* pso, const Camera* camera, Heap* heap,
                                const XMMATRIX& projMatrix, const PtConfig& config, float dirLightIntensity, XMFLOAT3 dirLightColor, XMFLOAT3 dirLightDir, int debugModeIdx)
{
    GPU_SCOPE(cmdList, L"Path Tracing Backend");

    const bool frameIncAllowed = config.MaxFrameNum == 0 || m_numFrames < config.MaxFrameNum;

    if (m_numFrames == 0)
    {
        m_accumTexture->Transition(cmdList, D3D12_RESOURCE_STATE_COPY_DEST);

        D3D12_TEXTURE_COPY_LOCATION dstLocation;
        dstLocation.pResource = m_accumTexture->GetD12Resource()->GetResource();
        dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        dstLocation.SubresourceIndex = 0;

        D3D12_TEXTURE_COPY_LOCATION srcLocation;
        srcLocation.pResource = m_accumClearBuffer->GetD12Resource()->GetResource();
        srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        srcLocation.SubresourceIndex = 0;

        D3D12_BOX srcBox;
        srcBox.front = 0;
        srcBox.back = 1;
        srcBox.left = 0;
        srcBox.right = Config::GetSystem().RtvWidth;
        srcBox.top = 0;
        srcBox.bottom = Config::GetSystem().RtvHeight;
        cmdList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, &srcBox);
    }

    m_accumTexture->Transition(cmdList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    {
        cmdList->SetGraphicsRootSignature(rootSig);
        cmdList->SetPipelineState(pso);

        CbvPathTracing cbv;
        cbv.CameraPositionWorld = camera->GetPosition();
        XMStoreFloat4x4(&cbv.InvP, XMMatrixInverse(nullptr, projMatrix));
        XMStoreFloat4x4(&cbv.InvV, XMMatrixInverse(nullptr, camera->GetViewMatrix()));
        cbv.NumBounces = config.NumBounces;
        cbv.RussianRouletteMinBounces = config.RussianRouletteMinBounces;
        cbv.SPP = config.SPP;
        cbv.NumFrames = m_numFrames;
        cbv.AccumulationEnabled = config.AccumulationEnabled ? 1u : 0u;
        cbv.WindowAppGuiWidth = Config::GetSystem().WindowAppGuiWidth;
        cbv.UpdateAccumulation = frameIncAllowed ? 1 : 0;
        cbv.FireflyThreshold = config.FireflyThreshold;
        cbv.DofFocalDist = config.DofFocalDist;
        cbv.DofLensRadius = config.DofLensRadius;

        cbv.DirLight = dirLightDir;
        cbv.DirLightColor = dirLightColor;
        cbv.DirLightCosAngularRadius = 1.0f - config.DirLightCosAngularRadius;
        cbv.DirLightIntensity = dirLightIntensity;

        cbv.Jitter = XMFLOAT2(0, 0);
        if (config.JitterEnabled)
        {
            const XMFLOAT2 texelSize = XMFLOAT2(1.0f / Config::GetSystem().RtvWidth, 1.0f / Config::GetSystem().RtvHeight);
            cbv.Jitter = XMFLOAT2((m_numFrames % 8) / 8.0f, ((m_numFrames / 8) % 8) / 8.0f);
            cbv.Jitter.x = (cbv.Jitter.x - 0.5f) * 2.0f;
            cbv.Jitter.y = (cbv.Jitter.y - 0.5f) * 2.0f;
            cbv.Jitter.x *= texelSize.x;
            cbv.Jitter.y *= texelSize.y;
        }

        m_material->UpdateCBV(0, &cbv);

        if (debugModeIdx != -1)
        {
            CbvPathTracingDebug cbvDebug;
            cbvDebug.DebugIdx = static_cast<DebugBuffer>(debugModeIdx);
            m_material->UpdateCBV(1, &cbvDebug);
        }

        m_material->SetDescriptorTables(cmdList);

        const CD3DX12_GPU_DESCRIPTOR_HANDLE bindlessHandle(heap->GetGPUHandle(), heap->GetBindlessTexBase(), heap->GetIncrementSize());
        cmdList->SetGraphicsRootDescriptorTable(2, bindlessHandle);

        cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        cmdList->IASetVertexBuffers(0, 1, &m_fullScreenTriangle.GetVertexBufferView());
        cmdList->IASetIndexBuffer(&m_fullScreenTriangle.GetIndexBufferView());
        cmdList->DrawIndexedInstanced(static_cast<UINT>(m_fullScreenTriangle.GetIndexCount()), 1, 0, 0, 0);
    }

    if (frameIncAllowed)
        m_numFrames++;
}

void PathTracingContext::Reset()
{
    m_numFrames = 0;
}
