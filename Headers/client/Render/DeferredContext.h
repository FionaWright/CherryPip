//
// Created by fiona on 15/01/2026.
//

#ifndef CHERRYPIP_DEFERREDCONTEXT_H
#define CHERRYPIP_DEFERREDCONTEXT_H
#include <d3d12.h>

#include "Scene.h"
#include "TextureRTV.h"


class DeferredContext
{
public:
    void Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
    void SetScene(Scene* scene);

    void Render(ID3D12GraphicsCommandList* cmdList);

private:
    TextureRTV m_rtvAlbedo, m_rtvNormal;
    Scene* m_scene = nullptr;
};


#endif //CHERRYPIP_DEFERREDCONTEXT_H