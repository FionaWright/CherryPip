//
// Created by fionaw on 09/11/2025.
//


#include "Render/RasterContext.h"
#include "CBV.h"
#include "Debug/GPUEventScoped.h"
#include "HWI/Heap.h"
#include "Render/Object.h"
#include "Render/Skybox.h"

void RasterContext::SetScene(Scene* scene)
{
    m_scene = scene;
}

void RasterContext::Render(const D3D* d3d, ID3D12GraphicsCommandList* cmdList, const XMMATRIX& vMatrix, const XMMATRIX& pMatrix, const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle, const Heap* heap, const Skybox* skybox) const
{
    GPU_SCOPE(cmdList, "Forward Backend");

    const float fRtvWidth = static_cast<float>(Config::GetSystem().RtvWidth);
    const float fRtvHeight = static_cast<float>(Config::GetSystem().RtvHeight);

    const CD3DX12_VIEWPORT viewport(0.0f, 0.0f, fRtvWidth, fRtvHeight);
    const CD3DX12_RECT scissorRect(0, 0, Config::GetSystem().RtvWidth, Config::GetSystem().RtvHeight);

    cmdList->RSSetViewports(1, &viewport);
    cmdList->RSSetScissorRects(1, &scissorRect);

    const CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(d3d->GetDsvHeapStart(), d3d->GetFrameIndex(), d3d->GetDsvDescriptorSize());
    cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    cmdList->ClearRenderTargetView(rtvHandle, Config::GetRender().RtvClearColor, 1, &scissorRect);
    cmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    CbvMatrices matrices = {};
    XMStoreFloat4x4(&matrices.V, vMatrix);
    XMStoreFloat4x4(&matrices.P, pMatrix);

    const CD3DX12_GPU_DESCRIPTOR_HANDLE bindlessHandle(heap->GetGPUHandle(), heap->GetBindlessTexBase(), heap->GetIncrementSize());

    {
        GPU_SCOPE(cmdList, "Skybox Pass");

        if (skybox)
            skybox->RenderForward(d3d, cmdList, vMatrix, pMatrix);
    }

    const auto& objects = m_scene->GetObjects();
    for (int i = 0; i < objects.size(); ++i)
    {
        GPU_SCOPE(cmdList, objects[i]->GetName());
        objects[i]->RenderForward(cmdList, matrices, bindlessHandle);
    }
}

