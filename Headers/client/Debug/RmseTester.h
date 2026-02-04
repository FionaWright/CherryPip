//
// Created by fiona on 03/02/2026.
//

#ifndef CHERRYPIP_RMSETESTER_H
#define CHERRYPIP_RMSETESTER_H
#include "ReadbackBuffer.h"
#include "HWI/Material.h"
#include "HWI/RootSig.h"
#include "HWI/Shader.h"
#include "HWI/Texture.h"


class D3D;

class RmseTester
{
public:
    void Init(D3D* d3d);

    void PrepareTakeSnapshot() { m_takeSnapshotNextFrame = true; }
    bool NeedTakeSnapshot() const { return m_takeSnapshotNextFrame; }
    void TakeSnapshot(D3D* d3d, uint32_t slot, D12Resource* finalRTV);

    [[nodiscard]] bool SlotAFilled() const { return m_slotAFilled; }
    [[nodiscard]] bool SlotBFilled() const { return m_slotBFilled; }

    void PrepareComputeRMSE() { m_computeRMSENextFrame = true; }
    bool NeedComputeRMSE() const { return m_computeRMSENextFrame; }
    void ComputeRMSE(D3D* d3d, Heap* heap);
    [[nodiscard]] float GetComputedRMSE() const { return m_lastComputedRMSE; }

    void BeginComputeGolden(uint32_t maxFrames, const char* path);
    [[nodiscard]] bool IsRunningGolden() const { return m_runningComputeGolden; }
    void UpdateComputeGolden(D3D* d3d, uint32_t currFrame, D12Resource* finalRTV);

    void SaveGolden(const uint8_t* data, size_t bufferSize, int width, int height) const;

    void PrepareLoadGolden(const char* path);
    [[nodiscard]] bool NeedLoadGolden() const { return m_loadGoldenNextFrame; }
    void LoadGolden(D3D* d3d, uint32_t slot);

    void BeginConvergenceTest(uint32_t maxFrames, const char* testName, uint32_t frameInc);
    void UpdateConvergenceTest(uint32_t currFrame);

private:
    Texture m_slotA, m_slotB;
    bool m_slotAFilled = false, m_slotBFilled = false;
    float m_lastComputedRMSE = -1.0f;

    bool m_takeSnapshotNextFrame = false;
    bool m_computeRMSENextFrame = false;
    bool m_runningComputeGolden = false, m_runningConvergenceTest = false;

    uint32_t m_goldenMaxFrames = 0;
    const char* m_goldenPath = nullptr;
    ReadbackBuffer m_goldenReadbackBuffer;
    bool m_loadGoldenNextFrame = false;

    RootSig m_rootSigSumSquaredErr;
    Shader m_shaderSumSquaredErr;
    Material m_matSumSquaredErr;
    D12Resource m_bufferSumSquaredErr;
    D12Resource m_readbackBufferSumSquaredErr;
};


#endif //CHERRYPIP_RMSETESTER_H