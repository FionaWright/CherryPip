//
// Created by fiona on 22/09/2025.
//

#ifndef PT_WIN32APP_H
#define PT_WIN32APP_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#include "Engine.h"
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

class Engine;
class App;

#include "Headers/Helper.h"

class Win32App
{
public:
    static int Run(App* pSample, HINSTANCE hInstance, int nCmdShow);
    static HWND GetHwnd() { return ms_hwnd; }

protected:
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    static HWND ms_hwnd;
    static std::unique_ptr<Engine> ms_engine;
};


#endif //PT_WIN32APP_H