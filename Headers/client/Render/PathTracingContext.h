//
// Created by fionaw on 28/10/2025.
//

#ifndef CHERRYPIP_PATHTRACINGCONTEXT_H
#define CHERRYPIP_PATHTRACINGCONTEXT_H
#include <d3d12.h>
#include <memory>
#include <vector>
#include <wrl/client.h>

#include "Camera.h"
#include "HWI/D12Resource.h"
#include "HWI/Model.h"

class RootSig;
class Shader;
class Material;
using Microsoft::WRL::ComPtr;

#include "PTBuffers.h"

class BLAS;
class TLAS;

class PathTracingContext
{
public:
    void Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const std::shared_ptr<TLAS>& tlas, const std::vector<std::shared_ptr<BLAS>>& blasList);
    void Render(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, ID3D12RootSignature* rootSig,
                ID3D12PipelineState* pso, const Camera* camera, const Material* material, const XMMATRIX& projMatrix) const;

    std::shared_ptr<D12Resource> GetInstanceDataBuffer() const { return m_instanceDataBuffer; }
    UINT GetNumInstances() const { return m_blasList.size(); }
    static size_t GetInstanceDataSize() { return sizeof(PtInstanceData); }

private:
    std::shared_ptr<TLAS> m_tlas;
    std::vector<std::shared_ptr<BLAS>> m_blasList;

    Model m_fullScreenTriangle;
    std::shared_ptr<Material> m_material;
    std::shared_ptr<Shader> m_shader;
    std::shared_ptr<RootSig> m_rootSig;

    std::vector<PtInstanceData> m_instanceDataList;
    std::shared_ptr<D12Resource> m_instanceDataBuffer;
};


#endif //CHERRYPIP_PATHTRACINGCONTEXT_H
