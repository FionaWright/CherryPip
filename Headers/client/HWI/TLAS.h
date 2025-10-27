//
// Created by fionaw on 27/10/2025.
//

#ifndef PT_TLAS_H
#define PT_TLAS_H
#include <memory>
#include <vector>

#include "D12Resource.h"


class BLAS;

class TLAS
{
public:
    void Init(ID3D12Device5* device, ID3D12GraphicsCommandList4* cmdList, const std::vector<std::shared_ptr<BLAS>>& blasList);

    ID3D12Resource* GetResource() const { return m_tlasResult.GetResource(); }
private:
    D12Resource m_tlasScratch, m_tlasResult;
    ComPtr<ID3D12Resource> m_instanceBuffer;
};


#endif //PT_TLAS_H