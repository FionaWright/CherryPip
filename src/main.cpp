#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

#include <windows.h>

#include <iostream>
#include <span>

#include "Apps/PathTracer/Headers/PathTracer.h"
#include "Apps/RasterViewer/Headers/RasterViewer.h"
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
    std::cout << "Console Window Initialised\n";

    Config::ParseCommandLineArgs(args);

    PathTracer pt;
    RasterViewer rv;

    std::vector<App*> apps;
    apps.emplace_back(&pt);
    apps.emplace_back(&rv);
    const int rslt = Win32App::Run(apps, hInstance, nCmdShow);

    CoUninitialize();

    return rslt;
}
