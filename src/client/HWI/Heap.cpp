//
// Created by fiona on 06/10/2025.
//

#include "HWI/Heap.h"

#include "Helper.h"

void Heap::Init(ID3D12Device* device, size_t numDescriptors)
{
    m_descriptorIncSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    m_heapSize = numDescriptors;

    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.NumDescriptors = numDescriptors;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    desc.NodeMask = 0;

    V(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_cbvSrvUavHeap)));
    V(m_cbvSrvUavHeap->SetName(L"CBV/SRV/UAV Heap"));
}

UINT Heap::GetNextDescriptor()
{
    if (m_currentHeapIndex == m_heapSize)
        throw std::exception("Heap is too smol :(");

    const UINT idx = m_currentHeapIndex;
    m_currentHeapIndex++;
    return idx;
}

void Heap::InitCBV(ID3D12Device* device, ID3D12Resource* resource, const size_t size, const UINT idx) const
{
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = resource->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = static_cast<UINT>(size);

    const auto cbvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_cbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart(), idx,
                                                   m_descriptorIncSize);
    device->CreateConstantBufferView(&cbvDesc, cbvHandle);
}

void Heap::InitSRV(ID3D12Device* device, ID3D12Resource* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc, const UINT idx) const
{
    const auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_cbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart(), idx,
                                                   m_descriptorIncSize);
    device->CreateShaderResourceView(resource, &desc, handle);
}

void Heap::SetHeap(ID3D12GraphicsCommandList* cmdList) const
{
    ID3D12DescriptorHeap* heap = m_cbvSrvUavHeap.Get();
    cmdList->SetDescriptorHeaps(1, &heap);
}
