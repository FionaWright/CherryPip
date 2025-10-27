//
// Created by fionaw on 27/10/2025.
//

#include "HWI/TLAS.h"

#include <d3d12.h>
#include <DirectXMath.h>

#include "Helper.h"
#include "HWI/BLAS.h"
using namespace DirectX;

void TLAS::Init(ID3D12Device5* device, ID3D12GraphicsCommandList4* cmdList, const std::vector<BLAS*>& blasList)
{
    const XMMATRIX worldMatrix = XMMatrixIdentity(); // TODO: Change to per object model matrix

    std::vector<D3D12_RAYTRACING_INSTANCE_DESC> instances;
    for (int i = 0; i < blasList.size(); i++)
    {
        D3D12_RAYTRACING_INSTANCE_DESC instance = {};
        XMStoreFloat3x4(reinterpret_cast<XMFLOAT3X4*>(&instance.Transform), XMMatrixTranspose(worldMatrix));

        instance.InstanceID = i;         // your custom ID
        instance.InstanceContributionToHitGroupIndex = 0;
        instance.InstanceMask = 0xFF;
        instance.AccelerationStructure = blasList[i]->GetResource()->GetGPUVirtualAddress();
        instance.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;

        instances.emplace_back(instance);
    }

    const UINT64 bufferSize = sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * instances.size();
    const CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
    const CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

    ComPtr<ID3D12Resource> instanceBuffer;
    V(device->CreateCommittedResource(
        &uploadHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&instanceBuffer)));

    void* mappedData = nullptr;
    V(instanceBuffer->Map(0, nullptr, &mappedData));
    memcpy(mappedData, instances.data(), bufferSize);
    instanceBuffer->Unmap(0, nullptr);


    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
    inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
    inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
    inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    inputs.NumDescs = static_cast<UINT>(instances.size());
    inputs.InstanceDescs = instanceBuffer->GetGPUVirtualAddress();

    // Compute GPU memory size needed
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo = {};
    device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &prebuildInfo);

    auto CreateBuffer = [&](const UINT64 size, const D3D12_RESOURCE_FLAGS flags)
    {
        constexpr D3D12_HEAP_PROPERTIES heapProps = { D3D12_HEAP_TYPE_DEFAULT };
        const D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(size, flags);

        ComPtr<ID3D12Resource> buffer;
        V(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc,
            D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, nullptr, IID_PPV_ARGS(&buffer)));

        return buffer;
    };

    m_tlasScratch = CreateBuffer(prebuildInfo.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    m_tlasResult  = CreateBuffer(prebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};
    buildDesc.Inputs = inputs;
    buildDesc.ScratchAccelerationStructureData = m_tlasScratch->GetGPUVirtualAddress();
    buildDesc.DestAccelerationStructureData = m_tlasResult->GetGPUVirtualAddress();

    cmdList->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    barrier.UAV.pResource = m_tlasResult.Get();
    cmdList->ResourceBarrier(1, &barrier);
}
