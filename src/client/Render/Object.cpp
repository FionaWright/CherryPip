//
// Created by fiona on 06/10/2025.
//

#include "Render/Object.h"

#include <iostream>

#include "CBV.h"

Object::~Object()
{
    std::cout << "Object Destroyed!" << std::endl;
}

void Object::Init(const std::shared_ptr<Transform>& transform,
                  const std::shared_ptr<Shader>& shader, const std::shared_ptr<RootSig>& rootSig,
                  const std::shared_ptr<Model>& model, const std::shared_ptr<Material>& mat)
{
    m_transform = transform;
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

    m_material->TransitionSrvsToPS(cmdList);
    m_material->SetDescriptorTables(cmdList);

    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmdList->IASetVertexBuffers(0, 1, &m_model->GetVertexBufferView());
    cmdList->IASetIndexBuffer(&m_model->GetIndexBufferView());
    cmdList->DrawIndexedInstanced(static_cast<UINT>(m_model->GetIndexCount()), 1, 0, 0, 0);
}
