//
// Created by fiona on 15/01/2026.
//

#ifndef CHERRYPIP_DENOISINGMANAGER_H
#define CHERRYPIP_DENOISINGMANAGER_H
#include <d3d12.h>

#include "HWI/D12Resource.h"
#include "HWI/Material.h"
#include "HWI/Model.h"
#include "HWI/RootSig.h"
#include "HWI/Shader.h"

class DenoisingManager
{
public:
    void Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, Heap* heap, D12Resource* tex);

    void DenoiseBox(ID3D12GraphicsCommandList* cmdList, const D12Resource* tex, uint32_t radius) const;

private:
    void initBox(ID3D12Device* device, Heap* heap, D12Resource* tex);

    Model m_fullScreenTriangle;
    Heap* m_pHeap = nullptr;

    RootSig m_rootSigBox;
    Shader m_shaderBox;
    Material m_matBox;
};


#endif //CHERRYPIP_DENOISINGMANAGER_H