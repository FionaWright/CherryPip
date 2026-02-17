//
// Created by fionaw on 09/11/2025.
//

#ifndef CHERRYPIP_READBACKMANAGER_H
#define CHERRYPIP_READBACKMANAGER_H
#include "Debug/ReadbackBuffer.h"


class TextureRTV;
class Heap;
class RootSig;
class Material;
class Shader;

struct Rgba8
{
    uint8_t r, g, b, a;
};

class ReadbackManager
{
public:
    void Init(const D3D* d3d, Heap* heap, TextureRTV* ptOut);

    void ReadbackPass(D3D* d3d, ID3D12GraphicsCommandList* cmdList, TextureRTV* inputRTV, bool readbackEveryFrame, XMFLOAT2 mousePosOnClick);
    void GUI(bool prevReadbackEnabled, bool& readbackEveryFrame);

    void SetInReadbackProcess(const bool inProcess) { m_inReadbackEveryFrameProcess = inProcess; }
    void ClearReadbackData() { m_readbackRgbaData.clear(); }

private:
    ReadbackBuffer m_readbackBuffer;
    bool m_inReadbackEveryFrameProcess = false;
    bool m_finishedReadingBack = true;
    std::shared_ptr<Shader> m_shaderReadbackHighlight;
    std::shared_ptr<Material> m_materialReadbackHighlight;
    std::shared_ptr<RootSig> m_rootSigReadbackHighlight;
    std::vector<Rgba8> m_readbackRgbaData;
};


#endif //CHERRYPIP_READBACKMANAGER_H
