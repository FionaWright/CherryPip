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

class PathTracer : public App
{
public:
    PathTracer();
    void OnInit(D3D* d3d) override;
    void OnUpdate(D3D* d3d, ID3D12GraphicsCommandList* cmdList) override;

    const WCHAR* GetTitle() const { return m_title.c_str(); }

    float m_AspectRatio;

private:
    static constexpr UINT c_FrameCount = 3;

    Heap m_heap;
    CameraController m_camera;

    std::shared_ptr<Material> m_material;
    std::shared_ptr<Shader> m_shader;
    std::shared_ptr<RootSig> m_rootSig;

    PathTracingContext m_ptContext;

    XMMATRIX m_projMatrix;

    // Window title.
    std::wstring m_title = L"Path Tracer";

    void loadAssets(D3D* d3d);
    void populateCommandList(const D3D* d3d, ID3D12GraphicsCommandList* cmdList);
};


#endif //PT_PATHTRACER_H
