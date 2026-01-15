//
// Created by fionaw on 05/11/2025.
//

#include "Render/TextureRTV.h"

#include <cassert>
#include <d3dx12.h>

#include "Debug/GPUEventScoped.h"
#include "System/Config.h"

void TextureRTV::Init(const LPCWSTR name, ID3D12Device* device, Heap* heap, const uint32_t width, const uint32_t height, const DXGI_FORMAT format)
{
    assert(heap->GetType() == D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDesc.Alignment = 0;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS|D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.Format = format;
    resourceDesc.Width = width;
    resourceDesc.Height = height;
    resourceDesc.MipLevels = 1;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;

    D3D12_CLEAR_VALUE clearVal = {};
    memcpy(clearVal.Color, Config::GetSystem().RtvClearColor, sizeof(float) * 4);
    clearVal.Format = resourceDesc.Format;

    m_d12Resource.Init(name, device, resourceDesc, D3D12_RESOURCE_STATE_RENDER_TARGET, &clearVal);

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
    rtvDesc.Format = resourceDesc.Format;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice = 0;
    rtvDesc.Texture2D.PlaneSlice = 0;

    m_heapIdx = heap->GetNextDescriptor();
    const auto rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(heap->GetCPUHandle(), m_heapIdx, heap->GetIncrementSize());

    device->CreateRenderTargetView(m_d12Resource.GetResource(), &rtvDesc, rtvHandle);
}
