//
// Created by fionaw on 27/10/2025.
//

#ifndef PT_BLAS_H
#define PT_BLAS_H
#include <memory>

#include "D12Resource.h"
#include "Render/Transform.h"

class Model;

class BLAS
{
public:
    void Init(ID3D12Device5* device, ID3D12GraphicsCommandList4* cmdList, const std::shared_ptr<Model>& model, const Transform& transform);

    const Transform& GetTransform() const { return m_transform; }
    ID3D12Resource* GetResource() const { return m_blasResult.GetResource(); }

private:
    std::shared_ptr<Model> m_model;
    Transform m_transform;
    D12Resource m_blasScratch;
    D12Resource m_blasResult;
};


#endif //PT_BLAS_H