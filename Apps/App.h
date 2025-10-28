//
// Created by fiona on 25/09/2025.
//

#ifndef PT_APP_H
#define PT_APP_H

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// Windows Runtime Library. Needed for ComPtr<> template class.
#include <wrl.h>

// DirectX 12 specific headers.
#include <d3d12.h>
#include <dxgi1_5.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
using namespace DirectX;

#include "d3dx12.h"

class D3D;

class App
{
public:
    virtual void OnInit(D3D* d3d);
    virtual void OnUpdate(D3D* d3d, ID3D12GraphicsCommandList* cmdList) = 0;

    virtual const char* GetName() const = 0;
    bool GetIsInitialized() const { return m_initialized; }

private:
    bool m_initialized = false;
};

#endif //PT_APP_H