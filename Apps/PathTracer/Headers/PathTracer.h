#ifndef PT_PATHTRACER_H
#define PT_PATHTRACER_H

#include "CBV.h"
#include "../../../Headers/client/System/App.h"
#include "HWI/Heap.h"
#include "HWI/RootSig.h"
#include "HWI/Shader.h"
#include "HWI/Texture.h"
#include "HWI/TLAS.h"
#include "Render/CameraController.h"
#include "Render/Object.h"
#include "Render/PathTracingContext.h"
#include "Render/TextureRTV.h"
#include "Render/Transform.h"

#ifdef _DEBUG
#include "FurnaceTest.h"
#include "ReadbackManager.h"
#endif

class BLAS;
using Microsoft::WRL::ComPtr;

enum PathTracerMode : uint32_t
{
    eStandard,
    eOutputBuffer,
    eFurnaceTestClassic,
    eFurnaceTestEmissive
};

struct PtConfig
{
    uint32_t SPP = 1;
    uint32_t NumBounces = 1;
    uint32_t MaxFrameNum = 0;
    PathTracerMode Mode = eStandard;
    DebugBuffer DebugBufferIdx = DebugBuffer::eNormalsShaded;
    bool AccumulationEnabled = true;
    bool JitterEnabled = false;
    bool ReadbackEnabled = false;
    bool ReadbackEveryFrame = false;
};

class PathTracer : public App
{
public:
    PathTracer();
    void OnInit(D3D* d3d) override;
    void OnUpdate(D3D* d3d, ID3D12GraphicsCommandList* cmdList) override;

    [[nodiscard]] const char* GetName() const override { return "Path Tracer"; }

    float m_AspectRatio;

private:
    void loadAssets(D3D* d3d);
    void populateCommandList(D3D* d3d, ID3D12GraphicsCommandList* cmdList);
    void GUI();

    Heap m_heap, m_heapRTV;
    CameraController m_camera;

    std::shared_ptr<Shader> m_shader, m_shaderDebug;
    std::shared_ptr<RootSig> m_rootSig, m_rootSigDebug;

    std::vector<D3D12_INPUT_ELEMENT_DESC> m_shaderILD;

    std::vector<std::shared_ptr<Scene>> m_scenes;
    uint32_t m_currentScene = 0;
    bool m_sceneDirty = true;

    PathTracingContext m_ptContext;
    PtConfig m_ptConfig;

    TextureRTV m_ptOutputTex;

    XMMATRIX m_projMatrix;

#ifdef _DEBUG
    FurnaceTest m_furnaceTest;
    ReadbackManager m_readbackManager;
#endif
};


#endif //PT_PATHTRACER_H
