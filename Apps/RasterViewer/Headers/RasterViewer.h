#ifndef PT_MODELLOADING_H
#define PT_MODELLOADING_H

#include "Apps/App.h"
#include "HWI/Heap.h"
#include "HWI/RootSig.h"
#include "HWI/Shader.h"
#include "HWI/Texture.h"
#include "Render/CameraController.h"
#include "Render/Object.h"
#include "Render/Transform.h"

using Microsoft::WRL::ComPtr;

class RasterViewer : public App
{
public:
    RasterViewer();
    void OnInit(D3D* d3d) override;
    void OnUpdate(D3D* d3d, ID3D12GraphicsCommandList* cmdList) override;

    const char* GetName() const override { return "Raster Viewer"; }

    float m_AspectRatio;

private:
    static constexpr UINT c_FrameCount = 3;

    Heap m_heap;
    CameraController m_camera;
    std::vector<std::shared_ptr<Object>> m_objects;

    void loadAssets(D3D* d3d);
    void populateCommandList(D3D* d3d, ID3D12GraphicsCommandList* cmdList);
};


#endif //PT_MODELLOADING_H
