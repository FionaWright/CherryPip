//
// Created by fiona on 03/02/2026.
//

#include "Debug/RmseTester.h"

#include "Buffers.h"
#include "CBV.h"
#include "Helper.h"
#include "spng.h"
#include "Debug/GPUEventScoped.h"
#include "Debug/PythonExecutor.h"
#include "HWI/D3D.h"
#include "HWI/Heap.h"
#include "System/FileHelper.h"

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
    }

    m_matSumSquaredErr.SetTex(d3d->GetDevice(), 0, heap, m_slotA.GetD12Resource());
    m_matSumSquaredErr.SetTex(d3d->GetDevice(), 1, heap, m_slotB.GetD12Resource());

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
    m_runningComputeGolden = true;
    m_maxFrames = maxFrames;
    m_taskName = path;
}

void RmseTester::UpdateComputeGolden(D3D* d3d, const uint32_t currFrame, D12Resource* finalRTV)
{
    if (currFrame < m_maxFrames || !m_runningComputeGolden)
        return;

    if (!m_goldenReadbackBuffer.IsInitialized())
    {
        m_goldenReadbackBuffer.Init(d3d, finalRTV);
    }
    m_goldenReadbackBuffer.Readback(d3d, finalRTV);
    const std::vector<uint8_t> readbackData = m_goldenReadbackBuffer.GetData();

    SaveGolden(readbackData.data(), readbackData.size(), finalRTV->GetDesc().Width, finalRTV->GetDesc().Height);
    m_runningComputeGolden = false;
}

void RmseTester::SaveGolden(const uint8_t* data, const size_t bufferSize, const int width, const int height) const
{
    const std::string filePath = wstringToString(ASSETS_SOURCE_DIR) + "/../Data/GoldenImages/" + m_taskName + ".png";

    const std::filesystem::path path = filePath;
    std::filesystem::create_directories(path.parent_path());

    FILE* file;
    fopen_s(&file, filePath.c_str(), "wb");
    if (!file)
        throw std::exception("I/O Error");

    int ret;
    spng_ctx* ctx = spng_ctx_new(SPNG_CTX_ENCODER);

    spng_ihdr ihdr = {};
    ihdr.width = width;
    ihdr.height = height;
    ihdr.color_type = SPNG_COLOR_TYPE_TRUECOLOR_ALPHA;
    ihdr.bit_depth = 8;
    ret = spng_set_ihdr(ctx, &ihdr);
    assert(ret == 0);
    ret = spng_set_png_file(ctx, file);
    assert(ret == 0);
    ret = spng_encode_image(ctx, data, bufferSize, SPNG_FMT_PNG, SPNG_ENCODE_FINALIZE);
    assert(ret == 0);
    spng_ctx_free(ctx);

    fclose(file);
}

void RmseTester::PrepareLoadGolden(const char* path)
{
    m_taskName = path;
    m_loadGoldenNextFrame = true;
}

void RmseTester::LoadGolden(D3D* d3d, const uint32_t slot)
{
    auto* slotTex = slot == 0 ? &m_slotA : &m_slotB;

    const std::string filePath = wstringToString(ASSETS_SOURCE_DIR) + "/../Data/GoldenImages/" + m_taskName + ".png";

    d3d->Flush();
    const auto cmdList = d3d->GetAvailableCmdList(D3D12_COMMAND_LIST_TYPE_DIRECT);
    slotTex->Init(d3d->GetDevice(), cmdList.Get(), filePath);
    V(cmdList->Close());
    d3d->ExecuteCommandList(cmdList.Get());
    d3d->Flush();

    if (slot == 0)
        m_slotAFilled = true;
    else
        m_slotBFilled = true;

    m_loadGoldenNextFrame = false;
}

void RmseTester::BeginConvergenceTest(const uint32_t maxFrames, const char* testName, const uint32_t frameInc)
{
    m_maxFrames = maxFrames;
    m_taskName = testName;
    m_frameIncrement = frameInc;
    m_lastFrameConvergenceTested = 0;
    m_rmses.clear();
    m_runningConvergenceTest = true;
}

void RmseTester::UpdateConvergenceTest(D3D* d3d, const uint32_t currFrame, Heap* heap, D12Resource* finalRTV)
{
    if (currFrame - m_lastFrameConvergenceTested < m_frameIncrement)
        return;

    TakeSnapshot(d3d, 1, finalRTV);
    ComputeRMSE(d3d, heap);
    m_rmses.emplace_back(m_lastComputedRMSE);
    m_lastFrameConvergenceTested = currFrame;

    if (currFrame < m_maxFrames)
        return;

    const std::string filePath = wstringToString(ASSETS_SOURCE_DIR) + "/../Data/RMSEs/" + m_taskName + ".csv";

    // Write RMSEs to file
    {
        const std::filesystem::path path = filePath;
        std::filesystem::create_directories(path.parent_path());

        std::fstream f;
        f.open(filePath.c_str(), std::ios::binary | std::fstream::out | std::ios::trunc);
        f.clear();
        f << "RMSE" << std::endl;
        for (int i = 0; i < m_rmses.size(); i++)
            f << std::to_string(m_rmses[i]) << std::endl;
        f.close();
    }

    // Plot graph
    {
        const std::vector<const char*> args = {
            filePath.c_str(),
            "--show",
            "--save"
        };
        PythonExecutor::ExecutePython("PlotConvergence.py", args);
    }

    m_runningConvergenceTest = false;
}

void RmseTester::CompareTests(const std::vector<std::string>& testNames)
{
    // Plot graph
    {
        std::vector<const char*> args;
        for (int i = 0; i < testNames.size(); i++)
        {
            const std::string filePath = "\"" + wstringToString(ASSETS_SOURCE_DIR) + "/../Data/RMSEs/" + testNames[i] + ".csv\"";
            args.push_back(_strdup(filePath.c_str()));
        }
        PythonExecutor::ExecutePython("PlotMultiConvergence.py", args);
    }
}
