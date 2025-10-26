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
    Engine(App* pSample, HWND hWnd, UINT windowWidth, UINT windowHeight);
    void Frame(HWND hWnd);
    void Render(App* pSample) const;
    void RenderGUI() const;

private:
    std::unique_ptr<D3D> m_d3d;
    HighResolutionClock m_clock;
};


#endif //PT_ENGINE_H