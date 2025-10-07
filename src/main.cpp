#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

#include <windows.h>

#include <iostream>
#include <span>

#include "Apps/HelloTriangle/Headers/HelloTriangle.h"
#include "Apps/SpinningCube/Headers/SpinningCube.h"
#include "Apps/TextureCube/Headers/TextureCube.h"
#include "Apps/ModelLoading/Headers/ModelLoading.h"
#include "System/Config.h"
#include "System/Win32App.h"

_Use_decl_annotations_

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR args, int nCmdShow)
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr))
        return -1;

    AllocConsole();
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    std::cout << "Debug output here\n";

    Config::ParseCommandLineArgs(args);

    ModelLoading sample;
    const int rslt = Win32App::Run(&sample, hInstance, nCmdShow);

    CoUninitialize();

    return rslt;
}
