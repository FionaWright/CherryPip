//
// Created by fionaw on 09/11/2025.
//

#include "System/CherryPip.h"

#include "System/Config.h"
#include "System/Win32App.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

int CherryPip::Run(App& app, HINSTANCE hInstance, const LPSTR args, const int nCmdShow)
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr))
        return -1;

#ifdef _DEBUG
    AllocConsole();
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    CherryPrint("Console Window Initialised");
#endif

    Config::ParseCommandLineArgs(args);

    const int result = Win32App::Run({&app}, hInstance, nCmdShow);

    CoUninitialize();

    return result;
}