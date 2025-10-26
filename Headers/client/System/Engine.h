//
// Created by fionaw on 26/10/2025.
//

#ifndef PT_ENGINE_H
#define PT_ENGINE_H
#include <memory>

#include "HighResolutionClock.h"
#include "Apps/App.h"

class D3D;

class Engine
{
public:
    Engine(App* pSample, HWND hWnd, UINT windowWidth);
    void Frame(HWND hWnd);

private:
    std::unique_ptr<D3D> m_d3d;
    HighResolutionClock m_clock;
};


#endif //PT_ENGINE_H