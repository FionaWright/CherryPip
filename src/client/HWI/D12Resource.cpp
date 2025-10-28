//
// Created by fiona on 30/09/2025.
//

#include "HWI/D12Resource.h"

#include <cassert>
#include <d3dx12_barriers.h>
#include <d3dx12_core.h>
#include <d3dx12_resource_helpers.h>

#include "Helper.h"

D12Resource::~D12Resource()
{
    std::cout << "Resource destroyed!" << std::endl;
}

void D12Resource::Init(const LPCWSTR name, ID3D12Device* device, const D3D12_RESOURCE_DESC& resourceDesc,
                       const D3D12_RESOURCE_STATES& initialState)
{
    const auto defaultHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    V(device->CreateCommittedResource(&defaultHeapProp, D3D12_HEAP_FLAG_NONE, &resourceDesc, initialState, nullptr,IID_PPV_ARGS(&m_resource)));
    V(m_resource->SetName(name));
    m_currentState = initialState;
    m_desc = resourceDesc;
}

void D12Resource::Init(const LPCWSTR name, ID3D12Device* device, const size_t size,
                       const D3D12_RESOURCE_STATES& initialState, const D3D12_RESOURCE_FLAGS flags)
{
    m_desc = CD3DX12_RESOURCE_DESC::Buffer(size, flags);
    const auto defaultHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    V(device->CreateCommittedResource(&defaultHeapProp, D3D12_HEAP_FLAG_NONE, &m_desc, initialState, nullptr,IID_PPV_ARGS(&m_resource)));
    V(m_resource->SetName(name));
    m_currentState = initialState;
}

void D12Resource::CreateHeap(ID3D12Device* device)
{
    // TODO: Shared upload heap?

    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_resource.Get(), 0, 1) * m_desc.DepthOrArraySize;

    const auto uploadHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    const auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
    V(device->CreateCommittedResource(&uploadHeapProp, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_uploadHeap)));
}

void D12Resource::UploadData(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const void* pData, const size_t totalBytes)
{
    if (!m_uploadHeap)
    {
        CreateHeap(device);
    }

    void* mappedData = nullptr;
    V(m_uploadHeap->Map(0, nullptr, &mappedData));
    memcpy(mappedData, pData, totalBytes);
    m_uploadHeap->Unmap(0, nullptr);

    cmdList->CopyBufferRegion(m_resource.Get(), 0, m_uploadHeap.Get(), 0, totalBytes);
}

void D12Resource::UploadTexture(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const uint8_t* pData, const size_t totalBytes,
                         const size_t rowPitch)
{
    assert(m_desc.DepthOrArraySize == 1);

    if (!m_uploadHeap)
    {
        CreateHeap(device);
    }

    constexpr int c_mip0 = 0;
    constexpr int c_slice0 = 0;
    const UINT subresourceIndex = D3D12CalcSubresource(c_mip0, c_slice0, 0, m_desc.MipLevels, m_desc.DepthOrArraySize);

    D3D12_SUBRESOURCE_DATA subresource = {};
    subresource.pData = pData;
    subresource.RowPitch = rowPitch;
    subresource.SlicePitch = totalBytes;

    constexpr UINT c_intermediateOffset = 0;
    UpdateSubresources(cmdList, m_resource.Get(), m_uploadHeap.Get(), c_intermediateOffset, subresourceIndex, 1,
                       &subresource);
    delete pData;
}

void D12Resource::UploadTexture(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const uint8_t** pData, const size_t totalBytes,
                         const size_t rowPitch)
{
    if (!m_uploadHeap)
    {
        CreateHeap(device);
    }

    UINT intermediateOffset = 0;

    for (int a = 0; a < m_desc.DepthOrArraySize; a++)
    {
        constexpr int c_mip0 = 0;
        const UINT subresourceIndex = D3D12CalcSubresource(c_mip0, a, 0, m_desc.MipLevels, m_desc.DepthOrArraySize);

        D3D12_SUBRESOURCE_DATA subresource = {};
        subresource.pData = pData[a];
        subresource.RowPitch = rowPitch;
        subresource.SlicePitch = totalBytes;

        UpdateSubresources(cmdList, m_resource.Get(), m_uploadHeap.Get(), intermediateOffset, subresourceIndex, 1,
                           &subresource);

        intermediateOffset += static_cast<UINT>(GetRequiredIntermediateSize(m_resource.Get(), subresourceIndex, 1));
    }

    for (int a = 0; a < m_desc.DepthOrArraySize; a++)
        delete[] pData[a];
}

void D12Resource::Transition(ID3D12GraphicsCommandList* cmdList, const D3D12_RESOURCE_STATES& newState,
                             const UINT subresourceIdx)
{
    if (m_currentState == newState)
        return;

    const CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_resource.Get(), m_currentState, newState, subresourceIdx);
    cmdList->ResourceBarrier(1, &barrier);
    m_currentState = newState;
}
