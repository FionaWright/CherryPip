//
// Created by fionaw on 03/11/2025.
//
#include "System/pch.h"
#include "Debug/ReadbackBuffer.h"

#include "../../../Headers/client/Helper.h"
#include "fastgltf/util.hpp"
#include "HWI/D3D.h"

void ReadbackBuffer::Init(const D3D* d3d, const D12Resource* resource)
{
    // Assumes Rgba8
    m_width = resource->GetDesc().Width;
    m_height = resource->GetDesc().Height;

    const D3D12_RESOURCE_DESC desc = resource->GetDesc();
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
    UINT numRows;
    d3d->GetDevice()->GetCopyableFootprints(&desc, 0, 1, 0,
        &footprint, &numRows, &m_rowPitch, &m_bufferSize);

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_READBACK;

    D3D12_RESOURCE_DESC readbackDesc = {};
    readbackDesc.DepthOrArraySize = 1;
    readbackDesc.Width = m_bufferSize;
    readbackDesc.Height = 1;
    readbackDesc.MipLevels = 1;
    readbackDesc.SampleDesc = { 1, 0};
    readbackDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    readbackDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    readbackDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;

    ComPtr<ID3D12Resource> bufferResource;

    V(d3d->GetDevice()->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &readbackDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&bufferResource)
    ));

    m_readbackBuffer.Fill(bufferResource, D3D12_RESOURCE_STATE_COPY_DEST);

    m_isInitialized = true;
}

void ReadbackBuffer::Readback(D3D* d3d, D12Resource* resource)
{
    assert(resource->GetDesc().Width == m_width && resource->GetDesc().Height == m_height);
    copyToBuffer(d3d, resource);

    std::vector<uint8_t> unpackedData;
    unpackedData.resize(m_bufferSize);

    void* mappedData = nullptr;
    const D3D12_RANGE readRange = {0, m_bufferSize};
    V(m_readbackBuffer.GetResource()->Map(0, &readRange, &mappedData));

    memcpy(unpackedData.data(), mappedData, m_bufferSize);

    constexpr D3D12_RANGE writeRange = {0, 0};
    m_readbackBuffer.GetResource()->Unmap(0, &writeRange);

    constexpr uint32_t channels = 4;
    const uint32_t bytesPerRow = m_width * channels;
    m_readbackData.clear();
    m_readbackData.resize(m_width * m_height * channels);

    const uint32_t alignedRowPitch = (m_rowPitch + 255) / 256 * 256;

    for (uint32_t y = 0; y < m_height; ++y)
    {
        uint8_t* dstAddr = m_readbackData.data() + y * bytesPerRow;
        const uint8_t* srcAddr = unpackedData.data() + y * alignedRowPitch;
        memcpy(dstAddr, srcAddr, bytesPerRow);
    }
}

void ReadbackBuffer::copyToBuffer(D3D* d3d, D12Resource* resource) const
{
    const auto cmdList = d3d->GetAvailableCmdList(D3D12_COMMAND_LIST_TYPE_DIRECT);

    const D3D12_RESOURCE_STATES prevState = resource->GetCurrentState();
    resource->Transition(cmdList.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE);

    D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
    srcLocation.pResource = resource->GetResource();
    srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    srcLocation.SubresourceIndex = 0;

    const D3D12_RESOURCE_DESC desc = resource->GetDesc();
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
    UINT numRows;
    UINT64 rowSizeInBytes, totalBytes;
    d3d->GetDevice()->GetCopyableFootprints(&desc, 0, 1, 0,
        &footprint, &numRows, &rowSizeInBytes, &totalBytes);

    D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
    dstLocation.pResource = m_readbackBuffer.GetResource();
    dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    dstLocation.PlacedFootprint = footprint;

    cmdList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);

    resource->Transition(cmdList.Get(), prevState); // Can this be gotten rid of through a refactor of the command list system?

    V(cmdList->Close());
    d3d->ExecuteCommandList(cmdList.Get());

    d3d->Flush();
}
