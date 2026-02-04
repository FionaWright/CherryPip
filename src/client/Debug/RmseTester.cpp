//
// Created by fiona on 03/02/2026.
//

#include "Debug/RmseTester.h"

#include "Helper.h"
#include "HWI/D3D.h"

void RmseTester::Init(D3D* d3d)
{
    m_slotA.InitEmpty(d3d->GetDevice(), Config::GetRender().RtvFormat, Config::GetSystem().RtvWidth, Config::GetSystem().RtvHeight);
    m_slotB.InitEmpty(d3d->GetDevice(), Config::GetRender().RtvFormat, Config::GetSystem().RtvWidth, Config::GetSystem().RtvHeight);
}

void RmseTester::TakeSnapshot(D3D* d3d, const uint32_t slot, const D12Resource* finalRTV)
{
    d3d->Flush();
    const auto cmdList = d3d->GetAvailableCmdList(D3D12_COMMAND_LIST_TYPE_DIRECT);

    D3D12_TEXTURE_COPY_LOCATION copyLocationDst{};
    copyLocationDst.pResource = slot == 0 ? m_slotA.GetD12Resource()->GetResource() : m_slotB.GetD12Resource()->GetResource();
    copyLocationDst.SubresourceIndex = 0;
    D3D12_TEXTURE_COPY_LOCATION copyLocationSrc{};
    copyLocationSrc.pResource = finalRTV->GetResource();
    copyLocationSrc.SubresourceIndex = 0;
    D3D12_BOX box{};
    box.left = 0;
    box.bottom = 0;
    box.back = 0;
    box.right = finalRTV->GetDesc().Width;
    box.top = finalRTV->GetDesc().Height;
    box.front = 1;

    cmdList->CopyTextureRegion(&copyLocationDst, 0, 0, 0, &copyLocationSrc, &box);

    V(cmdList->Close());
    d3d->ExecuteCommandList(cmdList.Get());
    d3d->Flush();

    if (slot == 0)
        m_slotAFilled = true;
    else
        m_slotBFilled = true;
}

void RmseTester::ComputeRMSE(D3D* d3d)
{
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
