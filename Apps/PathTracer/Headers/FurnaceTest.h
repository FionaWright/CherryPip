//
// Created by fionaw on 09/11/2025.
//

#ifndef CHERRYPIP_FURNACETEST_H
#define CHERRYPIP_FURNACETEST_H
#include "HWI/D3D.h"
#include "HWI/Shader.h"
#include "Render/PathTracingContext.h"

class FurnaceTest
{
public:
    void Init(D3D* d3d, ID3D12GraphicsCommandList* cmdList, const std::vector<D3D12_INPUT_ELEMENT_DESC>& ildVec, ID3D12RootSignature* rootSig, Heap* heap);

    bool GetIsInitialised() const { return m_initialised; }

    Material* GetMaterial() const { return m_materialFurnace.get(); }
    ID3D12PipelineState* GetShaderClassic() const { return m_shaderFurnaceClassic->GetPSO(); }
    ID3D12PipelineState* GetShaderEmissive() const { return m_shaderFurnaceEmissive->GetPSO(); }
    PathTracingContext* GetPtContext() { return &m_ptContextFurnaceTest; }

private:
    bool m_initialised = false;

    std::shared_ptr<Material> m_materialFurnace;
    std::shared_ptr<Shader> m_shaderFurnaceClassic, m_shaderFurnaceEmissive;

    PathTracingContext m_ptContextFurnaceTest;

    std::shared_ptr<Model> m_furnaceTestSphere;
};


#endif //CHERRYPIP_FURNACETEST_H