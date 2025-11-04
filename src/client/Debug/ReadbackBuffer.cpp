//
// Created by fionaw on 03/11/2025.
//

#include "Debug/ReadbackBuffer.h"

#include "Helper.h"
#include "HWI/D3D.h"

void ReadbackBuffer::Init(const D3D* d3d, ID3D12GraphicsCommandList* cmdList, Texture* texture)
{
    m_assignedTex = texture;

    // Assumes Rgba8
    assert(texture->GetDesc().Format == DXGI_FORMAT_R8G8B8A8_UNORM);
    m_bufferSize = m_assignedTex->GetDesc().Width * m_assignedTex->GetDesc().Height * sizeof(uint8_t) * 4;

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

    V(d3d->GetDevice()->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &readbackDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&m_readbackBuffer)
    ));
}

void ReadbackBuffer::Readback(D3D* d3d)
{
    auto cmdList = d3d->GetAvailableCmdList(D3D12_COMMAND_LIST_TYPE_DIRECT);

    const D3D12_RESOURCE_STATES prevState = m_assignedTex->GetD12Resource()->GetCurrentState();
    m_assignedTex->Transition(cmdList.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE);

    D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
    srcLocation.pResource = m_assignedTex->GetD12Resource()->GetResource();
    srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    srcLocation.SubresourceIndex = 0;

    const D3D12_RESOURCE_DESC desc = m_assignedTex->GetDesc();
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
    UINT numRows;
    UINT64 rowSizeInBytes, totalBytes;
    d3d->GetDevice()->GetCopyableFootprints(&desc, 0, 1, 0,
        &footprint, &numRows, &rowSizeInBytes, &totalBytes);

    D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
    dstLocation.pResource = m_readbackBuffer.Get();
    dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    dstLocation.PlacedFootprint.Offset = 0;
    dstLocation.PlacedFootprint.Footprint.Width = m_assignedTex->GetDesc().Width;
    dstLocation.PlacedFootprint.Footprint.Height = m_assignedTex->GetDesc().Height;
    dstLocation.PlacedFootprint.Footprint.Format = m_assignedTex->GetDesc().Format;
    dstLocation.PlacedFootprint.Footprint.Depth = 1;
    dstLocation.PlacedFootprint.Footprint.RowPitch = rowSizeInBytes;

    cmdList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);

    m_assignedTex->Transition(cmdList.Get(), prevState); // Can this be gotten rid of through a refactor of the command list system?

    V(cmdList->Close());
    d3d->ExecuteCommandList(cmdList.Get());

    d3d->Flush();

    void* mappedData = nullptr;
    const D3D12_RANGE readRange = {0, m_bufferSize};
    V(m_readbackBuffer->Map(0, &readRange, &mappedData));

    m_readbackData.clear();
    m_readbackData.resize(m_bufferSize);
    memcpy(m_readbackData.data(), mappedData, m_bufferSize);

    constexpr D3D12_RANGE writeRange = {0, 0};
    m_readbackBuffer->Unmap(0, &writeRange);
}
