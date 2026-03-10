#ifndef H_DEBUG_WINDOW_H
#define H_DEBUG_WINDOW_H

struct DebugWindow
{
// public:
    DebugWindow(float2 topLeft, float2 bottomRight);
    DebugWindow(float top, float left, float bottom, float right);
    bool Select(float2 uv);

    float2 m_UV = float2(-1, -1);

// private:
    float2 m_topLeft, m_bottomRight;
};

DebugWindow::DebugWindow(float2 topLeft, float2 bottomRight)
{
    m_topLeft = topLeft;
    m_bottomRight = bottomRight;
}

DebugWindow::DebugWindow(float top, float left, float bottom, float right)
{
    m_topLeft = float2(top, left);
    m_bottomRight = float2(bottom, right);
}

bool DebugWindow::Select(float2 uv)
{
    if (uv.x < m_topLeft.x || 
        uv.x > m_bottomRight.x ||
        uv.y < m_topLeft.y ||
        uv.y > m_bottomRight.y)
        return false;

    float2 range = m_bottomRight - m_topLeft;
    m_UV = (uv - m_topLeft) / range;
    return true;
}

// Usage:

// 2 Windows:
// Renders square texture (Non-stretched) between (0.2, 0) and (0.5, 0.3)
// And the texture value passed through a function Foo() between (0.5, 0) and (0.8, 0.3)
/*
DebugWindow w1(0.2, 0, 0.5, 0.3);
DebugWindow w2(0.5, 0, 0.8, 0.3);
if (w1.Select(input.uv))
    return tex.Sample(sampler, w1.m_UV);
else if (w2.Select(input.uv))
    return Foo(tex.Sample(sampler, w2.m_UV));
*/

// Recursive Window:
// Renders debug view in top left quadrant 
// Renders another view in upper corner of previous view 
/*
DebugWindow w1(0, 0, 0.5, 0.5);
DebugWindow w2(0, 0, 0.25, 0.25);
if (w1.Select(input.uv))
    if (w2.Select(w1.m_UV))
        return Foo();
    else
        return Bar();
*/

#endif