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
    void TakeSnapshot(uint32_t slot);

    void ComputeRMSE(D3D* d3d);
    [[nodiscard]] float GetComputedRMSE() const { return m_lastComputedRMSE; }

    void BeginComputeGolden(uint32_t maxFrames, const char* path);
    void UpdateComputeGolden(uint32_t currFrame);

    void BeginConvergenceTest(uint32_t maxFrames, const char* testName, uint32_t frameInc);
    void UpdateConvergenceTest(uint32_t currFrame);

private:
    Texture m_slotA, m_slotB;
    float m_lastComputedRMSE = -1.0f;
};


#endif //CHERRYPIP_RMSETESTER_H