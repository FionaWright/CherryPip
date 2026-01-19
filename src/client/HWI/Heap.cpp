//
// Created by fiona on 06/10/2025.
//

#include "System/pch.h"
#include "HWI/Heap.h"

#include "../../../Headers/client/Helper.h"
#include "HWI/Texture.h"

void Heap::Init(ID3D12Device* device, const size_t numDescriptors, const D3D12_DESCRIPTOR_HEAP_TYPE type)
{
    m_type = type;
    m_descriptorIncSize = device->GetDescriptorHandleIncrementSize(m_type);
    m_heapSize = numDescriptors;

    m_baseBindlessTex = m_heapSize / 2;
    m_currentHeapIndexBindlessTex = m_baseBindlessTex; // Should I give more control to the user?

    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = m_type;
    desc.NumDescriptors = numDescriptors;
    desc.Flags = m_type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    desc.NodeMask = 0;

    V(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_heapResource)));
    V(m_heapResource->SetName(L"Heap"));
}

UINT Heap::GetNextDescriptor()
{
    if (m_currentHeapIndex == m_heapSize)
        throw std::exception("Heap is too smol :(");

    const UINT idx = m_currentHeapIndex;
    m_currentHeapIndex++;
    return idx;
}

UINT Heap::GetNextDescriptorBindlessTexture()
{
    if (m_currentHeapIndexBindlessTex == m_heapSize)
        throw std::exception("Heap is too smol :(");

    const UINT idx = m_currentHeapIndexBindlessTex;
    m_currentHeapIndexBindlessTex++;
    return idx;
}

UINT Heap::AddBindlessTexture(ID3D12Device* device, std::shared_ptr<Texture> tex)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Format = tex->GetFormat();
    srvDesc.Texture2D.MipLevels = tex->GetDesc().MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.PlaneSlice = 0;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    const UINT idx = GetNextDescriptorBindlessTexture();
    InitSRV(device, tex->GetD12Resource()->GetResource(), srvDesc, idx);

    return idx - GetBindlessTexBase();
}

void Heap::InitCBV(ID3D12Device* device, ID3D12Resource* resource, const size_t size, const UINT idx) const
{
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = resource->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = static_cast<UINT>(size);

    const auto cbvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_heapResource->GetCPUDescriptorHandleForHeapStart(), idx,
                                                   m_descriptorIncSize);
    device->CreateConstantBufferView(&cbvDesc, cbvHandle);
}

void Heap::InitSRV(ID3D12Device* device, ID3D12Resource* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc, const UINT idx) const
{
    const auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_heapResource->GetCPUDescriptorHandleForHeapStart(), idx,
                                                   m_descriptorIncSize);
    device->CreateShaderResourceView(resource, &desc, handle);
}

void Heap::InitUAV(ID3D12Device* device, ID3D12Resource* resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, const UINT idx) const
{
    const auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_heapResource->GetCPUDescriptorHandleForHeapStart(), idx,
                                                   m_descriptorIncSize);
    device->CreateUnorderedAccessView(resource, nullptr, &desc, handle);
}

void Heap::SetHeap(ID3D12GraphicsCommandList* cmdList) const
{
    ID3D12DescriptorHeap* heap = m_heapResource.Get();
    cmdList->SetDescriptorHeaps(1, &heap);
}
