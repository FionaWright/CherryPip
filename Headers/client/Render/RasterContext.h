//
// Created by fionaw on 09/11/2025.
//

#ifndef CHERRYPIP_RASTERCONTEXT_H
#define CHERRYPIP_RASTERCONTEXT_H
#include "Scene.h"
#include "HWI/D3D.h"


class RasterContext
{
public:
    void SetScene(Scene* scene);
    void Render(const D3D* d3d, ID3D12GraphicsCommandList* cmdList, const XMMATRIX& vMatrix, const XMMATRIX& pMatrix) const;

private:
    Scene* m_scene = nullptr;
};


#endif //CHERRYPIP_RASTERCONTEXT_H