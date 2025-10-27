//
// Created by fionaw on 27/10/2025.
//

#ifndef PT_BLAS_H
#define PT_BLAS_H
#include "D12Resource.h"

class Model;

class BLAS
{
public:
    void Init(ID3D12Device5* device, ID3D12GraphicsCommandList4* cmdList, const Model* model);

    ID3D12Resource* GetResource() const { return m_blasResult.Get(); }

private:
    ComPtr<ID3D12Resource> m_blasScratch;
    ComPtr<ID3D12Resource> m_blasResult;
};


#endif //PT_BLAS_H