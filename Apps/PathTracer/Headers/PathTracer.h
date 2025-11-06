#ifndef PT_PATHTRACER_H
#define PT_PATHTRACER_H

#include "CBV.h"
#include "Apps/App.h"
#include "Debug/ReadbackBuffer.h"
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
    DebugBuffer DebugBufferIdx = DebugBuffer::eNormals;
    bool AccumulationEnabled = true;
    bool JitterEnabled = false;
    bool ReadbackEnabled = false;
    bool ReadbackEveryFrame = false;
};

struct Rgba8
{
    uint8_t r, g, b, a;
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
    void readbackPass(D3D* d3d, ID3D12GraphicsCommandList* cmdList);

    Heap m_heap, m_heapRTV;
    CameraController m_camera;

    std::shared_ptr<Material> m_material, m_materialDebug, m_materialFurnace;
    std::shared_ptr<Shader> m_shader, m_shaderDebug, m_shaderFurnaceClassic, m_shaderFurnaceEmissive;
    std::shared_ptr<RootSig> m_rootSig, m_rootSigDebug;

    std::vector<D3D12_INPUT_ELEMENT_DESC> m_shaderILD;

    PathTracingContext m_ptContext, m_ptContextFurnaceTest;
    PtConfig m_ptConfig;

    std::shared_ptr<Model> m_furnaceTestSphere;

    TextureRTV m_ptOutputTex;

    XMMATRIX m_projMatrix;

    ReadbackBuffer m_readbackBuffer;
    XMFLOAT2 m_mousePosOnClick = { -1, -1 };
    bool m_inReadbackEveryFrameProcess = false;
    bool m_finishedReadingBack = true;
    std::shared_ptr<Shader> m_shaderReadbackHighlight;
    std::shared_ptr<Material> m_materialReadbackHighlight;
    std::shared_ptr<RootSig> m_rootSigReadbackHighlight;

    std::vector<Rgba8> m_readbackRgbaData;
};


#endif //PT_PATHTRACER_H
