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

class ModelLoading : public App
{
public:
    ModelLoading();
    void OnInit(D3D* d3d) override;
    void OnUpdate(D3D* d3d, ID3D12GraphicsCommandList* cmdList) override;

    const WCHAR* GetTitle() const { return m_title.c_str(); }

    float m_AspectRatio;

private:
    static constexpr UINT c_FrameCount = 3;

    Heap m_heap;
    CameraController m_camera;
    std::vector<std::shared_ptr<Object>> m_objects;

    // Window title.
    std::wstring m_title = L"Model Loading";

    void loadAssets(D3D* d3d);
    void populateCommandList(const D3D* d3d, ID3D12GraphicsCommandList* cmdList) const;
};


#endif //PT_MODELLOADING_H
