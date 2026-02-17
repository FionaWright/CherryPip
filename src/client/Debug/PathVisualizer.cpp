//
// Created by fiona on 17/02/2026.
//

#include "Debug/PathVisualizer.h"

#include "Helper.h"

void PathVisualizer::Init(const D3D* d3d, ID3D12GraphicsCommandList* cmdList, const uint32_t maxSPP)
{
    m_maxSPP = maxSPP;
    m_bufferSize = sizeof(DebugPathVisualizationStruct) * maxSPP;
    m_structuredBuffer.InitBuffer(L"Path Visualizer Structured Buffer", d3d->GetDevice(), m_bufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    m_readbackBuffer.InitBuffer(L"Path Visualizer Readback Buffer", d3d->GetDevice(), m_bufferSize, D3D12_RESOURCE_FLAG_NONE, true);

    m_structuredBuffer.Transition(cmdList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
}

std::vector<DebugPathVisualizationStruct> PathVisualizer::ReadbackData(D3D* d3d)
{
    d3d->Flush();
    auto cmdList = d3d->GetAvailableCmdList(D3D12_COMMAND_LIST_TYPE_DIRECT);

    // Copy StructuredBuffer -> ReadbackBuffer
    {
        m_structuredBuffer.Transition(cmdList.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE);
        m_readbackBuffer.Transition(cmdList.Get(), D3D12_RESOURCE_STATE_COPY_DEST);
        cmdList->CopyBufferRegion(m_readbackBuffer.GetResource(), 0, m_structuredBuffer.GetResource(), 0, m_bufferSize);

        m_structuredBuffer.Transition(cmdList.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    }

    V(cmdList->Close());
    d3d->ExecuteCommandList(cmdList.Get());
    d3d->Flush();

    std::vector<DebugPathVisualizationStruct> readbackData;
    readbackData.resize(m_maxSPP);

    // Readback data
    {
        void* mappedData = nullptr;
        const D3D12_RANGE readRange = {0, m_bufferSize};
        V(m_readbackBuffer.GetResource()->Map(0, &readRange, &mappedData));

        memcpy(readbackData.data(), mappedData, m_bufferSize);

        constexpr D3D12_RANGE writeRange = {0, 0};
        m_readbackBuffer.GetResource()->Unmap(0, &writeRange);
    }
    return readbackData;
}
