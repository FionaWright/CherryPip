//
// Created by fionaw on 27/10/2025.
//

#include "HWI/BLAS.h"

#include <d3d12.h>
#include <d3dx12_core.h>

#include "Helper.h"
#include "HWI/Model.h"

void BLAS::Init(ID3D12Device5* device, ID3D12GraphicsCommandList4* cmdList, const Model* model)
{
    ID3D12Resource* vertexBuffer = model->GetVertexBuffer();
    ID3D12Resource* indexBuffer = model->GetIndexBuffer();

    D3D12_RAYTRACING_GEOMETRY_DESC geomDesc = {};
    geomDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
    geomDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

    geomDesc.Triangles.VertexBuffer.StartAddress = vertexBuffer->GetGPUVirtualAddress();
    geomDesc.Triangles.VertexBuffer.StrideInBytes = model->GetVertexInputSize();
    geomDesc.Triangles.VertexCount = model->GetVertexCount();
    geomDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;

    geomDesc.Triangles.IndexBuffer = indexBuffer->GetGPUVirtualAddress();
    geomDesc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
    geomDesc.Triangles.IndexCount = model->GetIndexCount();

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
    inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
    inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
    inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    inputs.NumDescs = 1;
    inputs.pGeometryDescs = &geomDesc;

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

    m_blasScratch = CreateBuffer(prebuildInfo.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    m_blasResult  = CreateBuffer(prebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};
    buildDesc.Inputs = inputs;
    buildDesc.ScratchAccelerationStructureData = m_blasScratch->GetGPUVirtualAddress();
    buildDesc.DestAccelerationStructureData = m_blasResult->GetGPUVirtualAddress();

    cmdList->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    barrier.UAV.pResource = m_blasResult.Get();
    cmdList->ResourceBarrier(1, &barrier);

}
