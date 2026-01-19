//
// Created by fiona on 22/09/2025.
//

#ifndef PT_WIN32APP_H
#define PT_WIN32APP_H

class Engine;
class App;

#include "../Helper.h"

class Win32App
{
public:
    static int Run(const std::vector<App*>& apps, HINSTANCE hInstance, int nCmdShow);
    static HWND GetHwnd() { return ms_hwnd; }

protected:
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    static HWND ms_hwnd;
    static std::unique_ptr<Engine> ms_engine;
};


#endif //PT_WIN32APP_H