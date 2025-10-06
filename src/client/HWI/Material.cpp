//
// Created by fiona on 06/10/2025.
//

#include "HWI/Material.h"

#include "Helper.h"
#include "HWI/Heap.h"

void Material::Init(const Heap* heap)
{
    m_cpuHandle = heap->GetCPUHandle();
    m_gpuHandle = heap->GetGPUHandle();
    m_descriptorIncSize = heap->GetIncrementSize();
}

void Material::AddCBV(ID3D12Device* device, Heap* heap, size_t size)
{
    const size_t alignedSize = (size + 255) & ~255; // Ceilings the size to the nearest 256

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC bufferDesc = {};
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Width = alignedSize;
    bufferDesc.Height = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels = 1;
    bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    const UINT idx = heap->GetNextDescriptor();
    CBV cbv = { nullptr, idx, size, alignedSize };

    V(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
                                    D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&cbv.Resource)));

    heap->InitCBV(device, cbv.Resource.Get(), alignedSize, idx);

    m_cbvs.push_back(cbv);
}

void Material::AddSRV(ID3D12Device* device, Heap* heap, Texture* tex)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = tex->GetFormat();
    srvDesc.Texture2D.MipLevels = tex->GetDesc().MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.PlaneSlice = 0;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    const UINT idx = heap->GetNextDescriptor();
    heap->InitSRV(device, tex->GetResource(), srvDesc, idx);

    m_srvs.push_back({idx});
}

void Material::UpdateCBV(const UINT regIdx, const void* data) const
{
    const CBV& cbv = m_cbvs[regIdx];

    void* dstData = nullptr;
    D3D12_RANGE readRange = {};
    V(cbv.Resource->Map(0, &readRange, &dstData));
    std::memcpy(dstData, data, cbv.Size);
    cbv.Resource->Unmap(0, nullptr);
}

// TODO: Not general
void Material::SetDescriptorTables(ID3D12GraphicsCommandList* cmdList) const
{
    const CD3DX12_GPU_DESCRIPTOR_HANDLE cbvHandle(m_gpuHandle, m_cbvs[0].HeapIndex,
                                            m_descriptorIncSize);
    cmdList->SetGraphicsRootDescriptorTable(0, cbvHandle);

    const CD3DX12_GPU_DESCRIPTOR_HANDLE srvHandle(m_gpuHandle, m_srvs[0].HeapIndex,
                                            m_descriptorIncSize);
    cmdList->SetGraphicsRootDescriptorTable(1, srvHandle);
}
