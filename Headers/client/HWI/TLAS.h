//
// Created by fionaw on 27/10/2025.
//

#ifndef PT_TLAS_H
#define PT_TLAS_H
#include <vector>

#include "D12Resource.h"


class BLAS;

class TLAS
{
public:
    void Init(ID3D12Device5* device, ID3D12GraphicsCommandList4* cmdList, const std::vector<BLAS*>& blasList);

    ID3D12Resource* GetResource() const { return m_tlasResult.Get(); }
private:
    ComPtr<ID3D12Resource> m_tlasScratch, m_tlasResult;
};


#endif //PT_TLAS_H