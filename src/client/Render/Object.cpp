//
// Created by fiona on 06/10/2025.
//
#include "System/pch.h"

#include "Render/Object.h"

#include "CBV.h"

Object::~Object()
{

}

void Object::Init(const char* name, const std::shared_ptr<Transform>& transform,
                  const std::shared_ptr<Shader>& shader, const std::shared_ptr<RootSig>& rootSig,
                  const std::shared_ptr<Model>& model, const std::shared_ptr<Material>& mat)
{
    m_name = name;
    m_transform = transform ? transform : std::make_shared<Transform>();
    m_shader = shader;
    m_rootSig = rootSig;
    m_model = model;
    m_material = mat;
}

void Object::SetParent(Object* parent)
{
    m_parent = parent;
}

void Object::Render(ID3D12GraphicsCommandList* cmdList, CbvMatrices& matrices, const CD3DX12_GPU_DESCRIPTOR_HANDLE& bindlessHandle) const
{
    // TODO: Why are these in here
    cmdList->SetGraphicsRootSignature(m_rootSig->Get());
    cmdList->SetPipelineState(m_shader->GetPSO());

    const XMMATRIX M = m_transform->GetModelMatrix();
    XMStoreFloat4x4(&matrices.M, M);
    XMStoreFloat4x4(&matrices.MTI, XMMatrixInverse(nullptr, XMMatrixTranspose(M)));
    m_material->UpdateCBV(0, &matrices);

    m_material->TransitionSrvsToPS(cmdList);
    m_material->SetDescriptorTables(cmdList);
    
    cmdList->SetGraphicsRootDescriptorTable(2, bindlessHandle);

    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmdList->IASetVertexBuffers(0, 1, &m_model->GetVertexBufferView());
    cmdList->IASetIndexBuffer(&m_model->GetIndexBufferView());
    cmdList->DrawIndexedInstanced(static_cast<UINT>(m_model->GetIndexCount()), 1, 0, 0, 0);
}
