//
// Created by fionaw on 27/10/2025.
//

#include "Debug/GPUEventScoped.h"

#ifdef _DEBUG
#include <WinPixEventRuntime/pix3.h>
#endif

GPUEventScoped::GPUEventScoped(ID3D12GraphicsCommandList* cmdList, LPCWSTR label)
{
#ifdef _DEBUG
    PIXBeginEvent(cmdList, 0, label);
    m_heldCmdList = cmdList;
#endif
}

GPUEventScoped::~GPUEventScoped()
{
#ifdef _DEBUG
    PIXEndEvent(m_heldCmdList);
#endif
}