//
// Created by fionaw on 14/11/2025.
//

#ifndef CHERRYPIP_ENVMAP_H
#define CHERRYPIP_ENVMAP_H
#include <string>

#include "HWI/Material.h"
#include "HWI/RootSig.h"
#include "HWI/Shader.h"
#include "HWI/Texture.h"


class EnvMap
{
public:
    void Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const std::wstring& filePath, float rotation, Heap* heap);
    void UpdateRotation(float rotation);

    D12Resource* GetPano() { return m_pano.GetD12Resource(); }
    D12Resource* GetEA() { return m_ea.GetD12Resource(); }

private:
    Texture m_pano, m_ea;

    RootSig m_rootSigPanoToEA;
    Shader m_shaderPanoToEA;
    Material m_matPanoToEA;
};


#endif //CHERRYPIP_ENVMAP_H