//
// Created by fiona on 15/01/2026.
//

#include "Render/DeferredContext.h"

#include "CBV.h"
#include "Render/Object.h"
#include "Render/Skybox.h"
#include "System/Config.h"

void DeferredContext::Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, Heap* heap)
{
    m_rtvAlbedo.Init(L"Albedo GBuffer", device, heap, Config::GetSystem().RtvWidth, Config::GetSystem().RtvHeight, DXGI_FORMAT_R8G8B8A8_UNORM);
    m_rtvNormal.Init(L"Normals GBuffer", device, heap, Config::GetSystem().RtvWidth, Config::GetSystem().RtvHeight, DXGI_FORMAT_R8G8B8A8_UNORM);
}

void DeferredContext::SetScene(Scene* scene)
{
    m_scene = scene;
}

void DeferredContext::Render(const D3D* d3d, ID3D12GraphicsCommandList* cmdList, const XMMATRIX& vMatrix, const XMMATRIX& pMatrix, const Skybox* skybox)
{
    const float fRtvWidth = static_cast<float>(Config::GetSystem().RtvWidth);
    const float fRtvHeight = static_cast<float>(Config::GetSystem().RtvHeight);

    const CD3DX12_VIEWPORT viewport(0.0f, 0.0f, fRtvWidth, fRtvHeight);
    const CD3DX12_RECT scissorRect(0, 0, Config::GetSystem().RtvWidth, Config::GetSystem().RtvHeight);

    cmdList->RSSetViewports(1, &viewport);
    cmdList->RSSetScissorRects(1, &scissorRect);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandles[] = {
        m_rtvAlbedo.GetCpuHandle(),
        m_rtvNormal.GetCpuHandle()
    };

    const CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(d3d->GetDsvHeapStart(), d3d->GetFrameIndex(), d3d->GetDsvDescriptorSize());
    cmdList->OMSetRenderTargets(_countof(rtvHandles), rtvHandles, FALSE, &dsvHandle);

    for (int i = 0; i < _countof(rtvHandles); i++)
        cmdList->ClearRenderTargetView(rtvHandles[i], Config::GetSystem().RtvClearColor, 1, &scissorRect);
    cmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    CbvMatrices matrices = {};
    matrices.V = vMatrix;
    matrices.P = pMatrix;

    if (skybox)
        skybox->Render(d3d, cmdList, vMatrix, pMatrix);

    const auto& objects = m_scene->GetObjects();
    for (int i = 0; i < objects.size(); ++i)
    {
        objects[i]->Render(cmdList, matrices);
    }
}
