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
    virtual void OnInit(D3D* d3d) = 0;
    virtual void OnUpdate(D3D* d3d) = 0;
    virtual void ParseCommandLineArgs(_In_reads_(argc) WCHAR* argv[], int argc) = 0;
};

#endif //PT_APP_H