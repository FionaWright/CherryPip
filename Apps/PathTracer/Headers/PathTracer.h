#ifndef PT_PATHTRACER_H
#define PT_PATHTRACER_H

#include "Apps/App.h"
#include "HWI/Heap.h"
#include "HWI/RootSig.h"
#include "HWI/Shader.h"
#include "HWI/Texture.h"
#include "HWI/TLAS.h"
#include "Render/CameraController.h"
#include "Render/Object.h"
#include "Render/PathTracingContext.h"
#include "Render/Transform.h"

class BLAS;
using Microsoft::WRL::ComPtr;

struct PtConfig
{
    uint32_t SPP = 1;
    uint32_t NumBounces = 2;
    uint32_t MaxFrameNum = 0;
    bool RngPaused = false;
    bool AccumulationEnabled = true;
    bool JitterEnabled = true;
};

class PathTracer : public App
{
public:
    PathTracer();
    void OnInit(D3D* d3d) override;
    void OnUpdate(D3D* d3d, ID3D12GraphicsCommandList* cmdList) override;

    const char* GetName() const override { return "Path Tracer"; }

    float m_AspectRatio;

private:
    Heap m_heap;
    CameraController m_camera;

    std::shared_ptr<Material> m_material;
    std::shared_ptr<Shader> m_shader;
    std::shared_ptr<RootSig> m_rootSig;

    PathTracingContext m_ptContext;
    PtConfig m_ptConfig;

    XMMATRIX m_projMatrix;

    void loadAssets(D3D* d3d);
    void populateCommandList(D3D* d3d, ID3D12GraphicsCommandList* cmdList);
    void GUI();
};


#endif //PT_PATHTRACER_H
