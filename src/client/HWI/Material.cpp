//
// Created by fiona on 06/10/2025.
//

#include "HWI/Material.h"

#include "Helper.h"
#include "HWI/Heap.h"
#include "HWI/TLAS.h"

Material::~Material()
{
    // TODO: Unmap cbv.MappedGpuPtrs
    std::cout << "Material Destroyed!" << std::endl;
}

void Material::Init(const Heap* heap)
{
    m_cpuHandle = heap->GetCPUHandle();
    m_gpuHandle = heap->GetGPUHandle();
    m_descriptorIncSize = heap->GetIncrementSize();
}

void Material::AddCBV(ID3D12Device* device, Heap* heap, const size_t size)
{
    const size_t alignedSize = (size + 255) & ~255; // Ceilings the size to the nearest 256

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

    const D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(alignedSize);

    const UINT idx = heap->GetNextDescriptor();
    CBV cbv = { nullptr, idx, size, alignedSize, nullptr };

    V(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
                                    D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&cbv.Resource)));
    V(cbv.Resource->SetName(L"CBV"));

    heap->InitCBV(device, cbv.Resource.Get(), alignedSize, idx);

    V(cbv.Resource->Map(0, nullptr, &cbv.MappedGpuPtr));

    m_cbvs.push_back(cbv);
}

void Material::AddSRV(ID3D12Device* device, Heap* heap, std::shared_ptr<Texture> tex)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Format = tex->GetFormat();
    srvDesc.Texture2D.MipLevels = tex->GetDesc().MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.PlaneSlice = 0;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    const UINT idx = heap->GetNextDescriptor();
    heap->InitSRV(device, tex->GetResource(), srvDesc, idx);

    SRV srv = { idx };
    srv.Texture = tex;
    m_srvs.push_back(srv);
}

void Material::AddBuffer(ID3D12Device* device, Heap* heap, std::shared_ptr<D12Resource> resource, const UINT numElements, const size_t stride)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.NumElements = numElements;
    srvDesc.Buffer.StructureByteStride = stride;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    const UINT idx = heap->GetNextDescriptor();
    heap->InitSRV(device, resource->GetResource(), srvDesc, idx);

    SRV srv = { idx };
    srv.Buffer = resource;
    m_srvs.push_back(srv);
}

void Material::AddTLAS(ID3D12Device* device, Heap* heap, std::shared_ptr<TLAS> tlas)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
    srvDesc.RaytracingAccelerationStructure.Location = tlas->GetResource()->GetGPUVirtualAddress();
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    const UINT idx = heap->GetNextDescriptor();
    heap->InitSRV(device, nullptr, srvDesc, idx);

    SRV srv = { idx };
    srv.TLAS = tlas;
    m_srvs.push_back(srv);
}

void Material::TransitionSrvsToPS(ID3D12GraphicsCommandList* cmdList) const
{
    for (int i = 0; i < m_srvs.size(); i++)
    {
        if (m_srvs[i].Texture)
        {
            m_srvs[i].Texture->Transition(cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        }
        else if (m_srvs[i].Buffer)
        {
            m_srvs[i].Buffer->Transition(cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        }
    }
}

void Material::UpdateCBV(const UINT regIdx, const void* data) const
{
    const CBV& cbv = m_cbvs[regIdx];
    std::memcpy(cbv.MappedGpuPtr, data, cbv.Size);
}

// TODO: Not general
void Material::SetDescriptorTables(ID3D12GraphicsCommandList* cmdList) const
{
    int paramIdx = 0;

    if (m_cbvs.size() > 0)
    {
        const CD3DX12_GPU_DESCRIPTOR_HANDLE cbvHandle(m_gpuHandle, m_cbvs[0].HeapIndex,
                                            m_descriptorIncSize);
        cmdList->SetGraphicsRootDescriptorTable(paramIdx, cbvHandle);
        paramIdx++;
    }

    if (m_srvs.size() > 0)
    {
        const CD3DX12_GPU_DESCRIPTOR_HANDLE srvHandle(m_gpuHandle, m_srvs[0].HeapIndex,
                                            m_descriptorIncSize);
        cmdList->SetGraphicsRootDescriptorTable(paramIdx, srvHandle);
        paramIdx++;
    }
}
