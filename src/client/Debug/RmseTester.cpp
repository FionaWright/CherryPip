//
// Created by fiona on 03/02/2026.
//

#include "Debug/RmseTester.h"

#include "Buffers.h"
#include "CBV.h"
#include "Helper.h"
#include "Debug/GPUEventScoped.h"
#include "HWI/D3D.h"
#include "HWI/Heap.h"

void RmseTester::Init(D3D* d3d)
{
    m_slotA.InitEmpty(d3d->GetDevice(), Config::GetRender().RtvFormat, Config::GetSystem().RtvWidth, Config::GetSystem().RtvHeight);
    m_slotB.InitEmpty(d3d->GetDevice(), Config::GetRender().RtvFormat, Config::GetSystem().RtvWidth, Config::GetSystem().RtvHeight);
}

void RmseTester::TakeSnapshot(D3D* d3d, const uint32_t slot, D12Resource* finalRTV)
{
    d3d->Flush();
    const auto cmdList = d3d->GetAvailableCmdList(D3D12_COMMAND_LIST_TYPE_DIRECT);

    D12Resource* slotResource = slot == 0 ? m_slotA.GetD12Resource() : m_slotB.GetD12Resource();

    slotResource->Transition(cmdList.Get(), D3D12_RESOURCE_STATE_COPY_DEST);
    finalRTV->Transition(cmdList.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE);

    D3D12_TEXTURE_COPY_LOCATION copyLocationDst{};
    copyLocationDst.pResource = slotResource->GetResource();
    copyLocationDst.SubresourceIndex = 0;
    D3D12_TEXTURE_COPY_LOCATION copyLocationSrc{};
    copyLocationSrc.pResource = finalRTV->GetResource();
    copyLocationSrc.SubresourceIndex = 0;
    D3D12_BOX box{};
    box.left = 0;
    box.top = 0;
    box.back = 1;
    box.right = finalRTV->GetDesc().Width;
    box.bottom = finalRTV->GetDesc().Height;
    box.front = 0;

    cmdList->CopyTextureRegion(&copyLocationDst, 0, 0, 0, &copyLocationSrc, &box);

    V(cmdList->Close());
    d3d->ExecuteCommandList(cmdList.Get());
    d3d->Flush();

    if (slot == 0)
        m_slotAFilled = true;
    else
        m_slotBFilled = true;

    m_takeSnapshotNextFrame = false;
}

void RmseTester::ComputeRMSE(D3D* d3d, Heap* heap)
{
    // 9x9 * 16x16 = 144x144
    constexpr float c_blockSize = 144.0f;

    const float fWidth = static_cast<float>(m_slotA.GetDesc().Width);
    const float fHeight = static_cast<float>(m_slotA.GetDesc().Height);
    const float maxDim = std::max(fWidth, fHeight);
    const size_t numThreadGroups1D = std::ceil(maxDim / c_blockSize);

    const size_t bufferNumElements = numThreadGroups1D * numThreadGroups1D;
    const size_t bufferSize = bufferNumElements * sizeof(SumSquaredErrorStruct);

    if (!m_shaderSumSquaredErr.GetPSO())
    {
        D3D12_STATIC_SAMPLER_DESC samplers[1];
        samplers[0] = {};
        samplers[0].Filter = D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        samplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        samplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        samplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        samplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        samplers[0].ShaderRegister = 0;

        m_rootSigSumSquaredErr.SmartInit(d3d->GetDevice(), 1, 2, 1, false, samplers, _countof(samplers));

        m_shaderSumSquaredErr.InitCs(L"Compute/SumSquaredErrorCS.hlsl", d3d->GetDevice(), m_rootSigSumSquaredErr.Get());

        m_bufferSumSquaredErr.InitBuffer(L"SumSquaredErr StructuredBuffer", d3d->GetDevice(), bufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
        m_readbackBufferSumSquaredErr.InitBuffer(L"SumSquaredErr ReadbackBuffer", d3d->GetDevice(), bufferSize, D3D12_RESOURCE_FLAG_NONE, true);

        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
        uavDesc.Buffer.FirstElement = 0;
        uavDesc.Buffer.NumElements = bufferNumElements;
        uavDesc.Buffer.StructureByteStride = sizeof(SumSquaredErrorStruct);
        uavDesc.Buffer.CounterOffsetInBytes = 0;
        uavDesc.Format = DXGI_FORMAT_UNKNOWN; // structured buffer
        uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

        m_matSumSquaredErr.Init(heap);
        m_matSumSquaredErr.AddCBV(d3d->GetDevice(), heap, sizeof(CbvMaxLumRedSearch), "CBV Max Luminance Reduction Search"); // TODO: rename
        m_matSumSquaredErr.AddUAV(d3d->GetDevice(), heap, m_bufferSumSquaredErr.GetResource(), uavDesc);
        m_matSumSquaredErr.SetTex(d3d->GetDevice(), 0, heap, m_slotA.GetD12Resource());
        m_matSumSquaredErr.SetTex(d3d->GetDevice(), 1, heap, m_slotB.GetD12Resource());
    }

    d3d->Flush();

    auto cmdList = d3d->GetAvailableCmdList(D3D12_COMMAND_LIST_TYPE_DIRECT);

    // Compute Dispatches
    {
        GPU_SCOPE(cmdList.Get(), "Compute Sum of Squared Errors");

        m_bufferSumSquaredErr.Transition(cmdList.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

        CbvMaxLumRedSearch cbv;
        cbv.TexelSize = XMFLOAT2(1.0f / fWidth, 1.0f / fHeight);
        m_matSumSquaredErr.UpdateCBV(0, &cbv);

        heap->SetHeap(cmdList.Get());
        cmdList->SetComputeRootSignature(m_rootSigSumSquaredErr.Get());
        cmdList->SetPipelineState(m_shaderSumSquaredErr.GetPSO());

        m_matSumSquaredErr.TransitionSrvsToPS(cmdList.Get());
        m_matSumSquaredErr.SetDescriptorTables(cmdList.Get(), true);

        cmdList->Dispatch(numThreadGroups1D, numThreadGroups1D, 1);
    }

    V(cmdList->Close());
    d3d->ExecuteCommandList(cmdList.Get());
    d3d->Flush();

    cmdList = d3d->GetAvailableCmdList(D3D12_COMMAND_LIST_TYPE_DIRECT);

    // Copy StructuredBuffer -> ReadbackBuffer
    {
        m_bufferSumSquaredErr.Transition(cmdList.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE);
        m_readbackBufferSumSquaredErr.Transition(cmdList.Get(), D3D12_RESOURCE_STATE_COPY_DEST);
        cmdList->CopyBufferRegion(m_readbackBufferSumSquaredErr.GetResource(), 0, m_bufferSumSquaredErr.GetResource(), 0, bufferSize);
    }

    V(cmdList->Close());
    d3d->ExecuteCommandList(cmdList.Get());
    d3d->Flush();

    std::vector<SumSquaredErrorStruct> readbackData;
    readbackData.resize(bufferNumElements);

    // Readback data
    {
        void* mappedData = nullptr;
        const D3D12_RANGE readRange = {0, bufferSize};
        V(m_readbackBufferSumSquaredErr.GetResource()->Map(0, &readRange, &mappedData));

        memcpy(readbackData.data(), mappedData, bufferSize);

        constexpr D3D12_RANGE writeRange = {0, 0};
        m_readbackBufferSumSquaredErr.GetResource()->Unmap(0, &writeRange);
    }

    // Sum readback
    float totalSum = 0.0f;
    for (int i = 0; i < bufferNumElements; i++)
        totalSum += readbackData[i].SquaredError;

    // Compute RMSE
    float N = m_slotA.GetDesc().Width * m_slotA.GetDesc().Height;
    float MSE = totalSum / N;
    m_lastComputedRMSE = sqrt(MSE);
    CherryPrint("RMSE " << m_lastComputedRMSE);

    m_computeRMSENextFrame = false;
}

void RmseTester::BeginComputeGolden(uint32_t maxFrames, const char* path)
{
}

void RmseTester::UpdateComputeGolden(uint32_t currFrame)
{
}

void RmseTester::BeginConvergenceTest(uint32_t maxFrames, const char* testName, uint32_t frameInc)
{
}

void RmseTester::UpdateConvergenceTest(uint32_t currFrame)
{
}
