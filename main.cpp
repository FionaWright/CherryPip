#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

#include <windows.h>

#include <iostream>

#include "HelloTriangle.h"
#include "Win32App.h"

_Use_decl_annotations_

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr)) return -1;

    AllocConsole();
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    std::cout << "Debug output here\n";

    HMODULE dxCompilerDLL = LoadLibrary("dxcompiler.dll");
    if (!dxCompilerDLL) {
        DWORD err = GetLastError();
        std::cout << "LoadLibrary failed: " << err << "\n";
    } else {
        std::cout << "dxcompiler.dll loaded successfully!\n";
    }

    // Get DxcCreateInstance function
    auto DxcCreateInstanceFn = reinterpret_cast<HRESULT(__stdcall*)(REFCLSID, REFIID, LPVOID*)>(
        GetProcAddress(dxCompilerDLL, "DxcCreateInstance"));
    if (!DxcCreateInstanceFn) {
        std::cerr << "Failed to get DxcCreateInstance\n";
        return 1;
    }

    ComPtr<IDxcCompiler3> compiler;
    hr = DxcCreateInstanceFn(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
    if (FAILED(hr))
    {
        std::cout << "DxcCreateInstance failed: 0x" << std::hex << hr << "\n";
    }
    else
    {
        std::cout << "DXC loaded successfully!\n";
    }

    HelloTriangle sample;
    int rslt = Win32App::Run(&sample, hInstance, nCmdShow);

    CoUninitialize();

    return rslt;
}
