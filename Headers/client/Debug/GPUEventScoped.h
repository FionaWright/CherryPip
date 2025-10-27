//
// Created by fionaw on 27/10/2025.
//

#ifndef PT_GPUEVENTSCOPED_H
#define PT_GPUEVENTSCOPED_H
#include "Apps/App.h"

#define GPU_SCOPE(cmdList, label) GPUEventScoped CONCAT(scope_, __LINE__)(cmdList, label)
#define CONCAT(a, b) a##b

class GPUEventScoped
{
public:
    GPUEventScoped(ID3D12GraphicsCommandList* cmdList, LPCWSTR label);
    ~GPUEventScoped();

private:
    ID3D12GraphicsCommandList* m_heldCmdList;
};


#endif //PT_GPUEVENTSCOPED_H