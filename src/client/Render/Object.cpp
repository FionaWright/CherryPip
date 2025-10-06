//
// Created by fiona on 06/10/2025.
//

#include "Render/Object.h"

#include "DualIncludes/CBV.h"

void Object::Init(const std::shared_ptr<Transform>& transform, const std::shared_ptr<Texture>& tex,
                  const std::shared_ptr<Shader>& shader, const std::shared_ptr<RootSig>& rootSig,
                  const std::shared_ptr<Model>& model, const std::shared_ptr<Material>& mat)
{
    m_transform = transform;
    m_texture = tex;
    m_shader = shader;
    m_rootSig = rootSig;
    m_model = model;
    m_material = mat;
}

void Object::SetParent(Object* parent)
{
    m_parent = parent;
}

void Object::Render(ID3D12GraphicsCommandList* cmdList, CbvMatrices& matrices) const
{
    cmdList->SetGraphicsRootSignature(m_rootSig->Get());
    cmdList->SetPipelineState(m_shader->GetPSO());

    matrices.M = m_transform->GetModelMatrix(); // TODO: Cache
    matrices.MTI = XMMatrixInverse(nullptr, XMMatrixTranspose(matrices.M));

    m_material->UpdateCBV(0, &matrices);

    m_material->SetDescriptorTables(cmdList);

    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    //cmdList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    //cmdList->DrawInstanced(m_vertexCount, 1, 0, 0);
}
