//
// Created by fiona on 06/10/2025.
//

#include "System/pch.h"
#include "HWI/Material.h"

#include "Helper.h"
#include "HWI/Heap.h"
#include "HWI/TLAS.h"
#include "System/Config.h"

Material::~Material()
{
    // TODO: Unmap cbv.MappedGpuPtrs
    CherryPrint("Material Destroyed!");
}

void Material::Init(const Heap* heap, bool hasBindlessParam)
{
    m_cpuHandle = heap->GetCPUHandle();
    m_gpuHandle = heap->GetGPUHandle();
    m_descriptorIncSize = heap->GetIncrementSize();

    m_hasBindlessParam = hasBindlessParam;
}

void Material::AddCBV(ID3D12Device* device, Heap* heap, const size_t size, const char* debugName)
{
    const size_t alignedSize = (size + 255) & ~255; // Ceilings the size to the nearest 256

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

    const D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(alignedSize);

    const UINT idx = heap->GetNextDescriptor(debugName);
    CBV cbv = { nullptr, idx, size, alignedSize, nullptr };

    V(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
                                    D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&cbv.Resource)));
    V(cbv.Resource->SetName(L"CBV"));

    heap->InitCBV(device, cbv.Resource.Get(), alignedSize, idx);

    V(cbv.Resource->Map(0, nullptr, &cbv.MappedGpuPtr));

    m_cbvs.push_back(cbv);
}

void Material::SetSRV(ID3D12Device* device, const UINT srvIdx, Heap* heap, D12Resource* d12Resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc, const char* debugName)
{
    if (srvIdx > m_srvs.size())
        throw std::exception("Invalid srv index");

    ID3D12Resource* resource = d12Resource ? d12Resource->GetResource() : nullptr;

    if (srvIdx == m_srvs.size())
    {
        const UINT idx = heap->GetNextDescriptor(debugName);
        heap->InitSRV(device, resource, desc, idx);

        SRV srv = { idx };
        srv.Resource = d12Resource;
        m_srvs.push_back(srv);
        return;
    }

    SRV& srv = m_srvs.at(srvIdx);
    srv.Resource = d12Resource;
    heap->InitSRV(device, resource, desc, srv.HeapIndex);
}

void Material::SetTex(ID3D12Device* device, const UINT srvIdx, Heap* heap, std::shared_ptr<Texture> tex)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Format = tex->GetFormat();
    srvDesc.Texture2D.MipLevels = tex->GetDesc().MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.PlaneSlice = 0;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    m_tempTextureOwnership.push_back(tex);

    std::string debugNameStr = Config::GetSystem().DebugHeapEnabled ? wstringToString(tex->GetD12Resource()->GetDebugName()).c_str() : "";
    const char* debugName = Config::GetSystem().DebugHeapEnabled ? debugNameStr.c_str() : nullptr;
    SetSRV(device, srvIdx, heap, tex->GetD12Resource(), srvDesc, debugName);
}

void Material::SetTex(ID3D12Device* device, const UINT srvIdx, Heap* heap, D12Resource* d12Resource)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Format = d12Resource->GetDesc().Format;
    srvDesc.Texture2D.MipLevels = d12Resource->GetDesc().MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.PlaneSlice = 0;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    std::string debugNameStr = Config::GetSystem().DebugHeapEnabled ? wstringToString(d12Resource->GetDebugName()).c_str() : "";
    const char* debugName = Config::GetSystem().DebugHeapEnabled ? debugNameStr.c_str() : nullptr;
    SetSRV(device, srvIdx, heap, d12Resource, srvDesc, debugName);
}

void Material::SetBuffer(ID3D12Device* device, const UINT srvIdx, Heap* heap, std::shared_ptr<D12Resource> resource, const UINT numElements, const size_t stride)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.NumElements = numElements;
    srvDesc.Buffer.StructureByteStride = stride;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    m_tempResourceOwnership.push_back(resource);

    std::string debugNameStr = Config::GetSystem().DebugHeapEnabled ? wstringToString(resource->GetDebugName()).c_str() : "";
    const char* debugName = Config::GetSystem().DebugHeapEnabled ? debugNameStr.c_str() : nullptr;
    SetSRV(device, srvIdx, heap, resource.get(), srvDesc, debugName);
}

void Material::SetTlas(ID3D12Device* device, const UINT srvIdx, Heap* heap, const std::shared_ptr<TLAS>& tlas)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
    srvDesc.RaytracingAccelerationStructure.Location = tlas->GetResource()->GetGPUVirtualAddress();
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    m_tempTlasOwnership.push_back(tlas);

    SetSRV(device, srvIdx, heap, nullptr, srvDesc);
}

void Material::AddUAV(ID3D12Device* device, Heap* heap, const std::shared_ptr<Texture>& tex)
{
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Format = tex->GetFormat();
    uavDesc.Texture2D.MipSlice = 0;
    uavDesc.Texture2D.PlaneSlice = 0;

    std::string debugNameStr = Config::GetSystem().DebugHeapEnabled ? wstringToString(tex->GetD12Resource()->GetDebugName()).c_str() : "";
    const char* debugName = Config::GetSystem().DebugHeapEnabled ? debugNameStr.c_str() : nullptr;

    const UINT idx = heap->GetNextDescriptor(debugName);
    heap->InitUAV(device, tex->GetD12Resource()->GetResource(), uavDesc, idx);

    m_tempTextureOwnership.push_back(tex);

    UAV uav = { idx };
    m_uavs.push_back(uav);
}

void Material::AddUAV(ID3D12Device* device, Heap* heap, ID3D12Resource* resource, DXGI_FORMAT format)
{
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Format = format;
    uavDesc.Texture2D.MipSlice = 0;
    uavDesc.Texture2D.PlaneSlice = 0;

    const UINT idx = heap->GetNextDescriptor("UAV");
    heap->InitUAV(device, resource, uavDesc, idx);

    const UAV uav = { idx };
    m_uavs.push_back(uav);
}

void Material::AddUAV(ID3D12Device* device, Heap* heap, ID3D12Resource* resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc)
{
    const UINT idx = heap->GetNextDescriptor("UAV");
    heap->InitUAV(device, resource, desc, idx);

    const UAV uav = { idx };
    m_uavs.push_back(uav);
}

void Material::TransitionSrvsToPS(ID3D12GraphicsCommandList* cmdList) const
{
    for (int i = 0; i < m_srvs.size(); i++)
    {
        if (m_srvs[i].Resource)
        {
            m_srvs[i].Resource->Transition(cmdList, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
        }
    }
}

void Material::UpdateCBV(const UINT regIdx, const void* data) const
{
    if (regIdx >= m_cbvs.size())
        throw std::exception("You made a mistake :(");

    const CBV& cbv = m_cbvs[regIdx];
    std::memcpy(cbv.MappedGpuPtr, data, cbv.Size);
}

// TODO: Not general
void Material::SetDescriptorTables(ID3D12GraphicsCommandList* cmdList, const bool isCompute) const
{
    int paramIdx = 0;

    if (m_cbvs.size() > 0)
    {
        const CD3DX12_GPU_DESCRIPTOR_HANDLE cbvHandle(m_gpuHandle, m_cbvs[0].HeapIndex,
                                            m_descriptorIncSize);
        if (isCompute)
            cmdList->SetComputeRootDescriptorTable(paramIdx, cbvHandle);
        else
            cmdList->SetGraphicsRootDescriptorTable(paramIdx, cbvHandle);
        paramIdx++;
    }

    if (m_srvs.size() > 0)
    {
        const CD3DX12_GPU_DESCRIPTOR_HANDLE srvHandle(m_gpuHandle, m_srvs[0].HeapIndex,
                                            m_descriptorIncSize);
        if (isCompute)
            cmdList->SetComputeRootDescriptorTable(paramIdx, srvHandle);
        else
            cmdList->SetGraphicsRootDescriptorTable(paramIdx, srvHandle);
        paramIdx++;
    }

    if (m_hasBindlessParam)
        paramIdx++;

    if (m_uavs.size() > 0)
    {
        const CD3DX12_GPU_DESCRIPTOR_HANDLE uavHandle(m_gpuHandle, m_uavs[0].HeapIndex,
                                            m_descriptorIncSize);
        if (isCompute)
            cmdList->SetComputeRootDescriptorTable(paramIdx, uavHandle);
        else
            cmdList->SetGraphicsRootDescriptorTable(paramIdx, uavHandle);
        paramIdx++;
    }
}
