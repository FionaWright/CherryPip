//
// Created by fiona on 22/09/2025.
//

#ifndef PT_WIN32APP_H
#define PT_WIN32APP_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

#include <windows.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"

#include <string>
#include <wrl.h>
#include <shellapi.h>
#include <memory>

class D3D;
class App;

#include "Headers/Helper.h"

class Win32App
{
public:
    static int Run(App* pSample, HINSTANCE hInstance, int nCmdShow);
    static HWND GetHwnd() { return m_hwnd; }

protected:
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    static HWND m_hwnd;
    static std::unique_ptr<D3D> m_d3d;
};


#endif //PT_WIN32APP_H