//
// Created by fionaw on 27/10/2025.
//

#include "HWI/BLAS.h"

#include <d3d12.h>
#include <d3dx12_core.h>

#include "Helper.h"
#include "HWI/Model.h"

void BLAS::Init(ID3D12Device5* device, ID3D12GraphicsCommandList4* cmdList, const std::shared_ptr<Model>& model, const Transform& transform)
{
    m_transform = transform;
    m_model = model;

    ID3D12Resource* vertexBuffer = model->GetVertexBuffer()->GetResource();
    ID3D12Resource* indexBuffer = model->GetIndexBuffer()->GetResource();

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

    m_blasScratch.Init(L"BLAS Scratch", device, prebuildInfo.ScratchDataSizeInBytes, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    m_blasResult.Init(L"BLAS Result", device, prebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};
    buildDesc.Inputs = inputs;
    buildDesc.ScratchAccelerationStructureData = m_blasScratch.GetResource()->GetGPUVirtualAddress();
    buildDesc.DestAccelerationStructureData = m_blasResult.GetResource()->GetGPUVirtualAddress();

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
    barrier.Transition.pResource = vertexBuffer;
    cmdList->ResourceBarrier(1, &barrier);
    barrier.Transition.pResource = indexBuffer;
    cmdList->ResourceBarrier(1, &barrier);

    cmdList->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    barrier.UAV.pResource = m_blasResult.GetResource();
    cmdList->ResourceBarrier(1, &barrier);
}
