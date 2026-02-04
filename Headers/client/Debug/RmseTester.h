//
// Created by fiona on 03/02/2026.
//

#ifndef CHERRYPIP_RMSETESTER_H
#define CHERRYPIP_RMSETESTER_H
#include "HWI/Texture.h"


class D3D;

class RmseTester
{
public:
    void Init(D3D* d3d);

    void PrepareTakeSnapshot() { m_takeSnapshotNextFrame = true; }
    bool NeedTakeSnapshot() const { return m_takeSnapshotNextFrame; }
    void TakeSnapshot(D3D* d3d, uint32_t slot, const D12Resource* finalRTV);

    [[nodiscard]] bool SlotAFilled() const { return m_slotAFilled; }
    [[nodiscard]] bool SlotBFilled() const { return m_slotBFilled; }

    void ComputeRMSE(D3D* d3d);
    [[nodiscard]] float GetComputedRMSE() const { return m_lastComputedRMSE; }

    void BeginComputeGolden(uint32_t maxFrames, const char* path);
    void UpdateComputeGolden(uint32_t currFrame);

    void BeginConvergenceTest(uint32_t maxFrames, const char* testName, uint32_t frameInc);
    void UpdateConvergenceTest(uint32_t currFrame);

private:
    Texture m_slotA, m_slotB;
    bool m_slotAFilled = false, m_slotBFilled = false;
    float m_lastComputedRMSE = -1.0f;

    bool m_takeSnapshotNextFrame = false;
    bool m_runningComputeGolden = false, m_runningConvergenceTest = false;
};


#endif //CHERRYPIP_RMSETESTER_H