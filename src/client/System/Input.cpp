//
// Created by fionaw on 25/09/2025.
//

#include <windows.h>

#include "System/Input.h"

unordered_map<KeyCode::Key, bool> Input::ms_currentlyHeldKeys;
unordered_map<KeyCode::Key, bool> Input::ms_keysLastFrame;

XMFLOAT2 Input::ms_mousePos;
XMFLOAT2 Input::ms_mousePosLastFrame;

XMFLOAT2 Input::ms_mousePosClient;
XMFLOAT2 Input::ms_mousePosClientLastFrame;

float Input::ms_mouseWheelDelta;

bool Input::ms_mouseLeftState;
bool Input::ms_mouseRightState;
bool Input::ms_mouseMiddleState;

bool Input::ms_mouseLeftStateLastFrame;
bool Input::ms_mouseRightStateLastFrame;
bool Input::ms_mouseMiddleStateLastFrame;

void Input::AddKey(KeyCode::Key key)
{
    ms_currentlyHeldKeys.insert_or_assign(key, true);
}

void Input::RemoveKey(KeyCode::Key key)
{
    ms_currentlyHeldKeys.insert_or_assign(key, false);
}

void Input::SetMouseLeftState(bool state)
{
    ms_mouseLeftState = state;
}

void Input::SetMouseRightState(bool state)
{
    ms_mouseRightState = state;
}

void Input::SetMouseMiddleState(bool state)
{
    ms_mouseMiddleState = state;
}

void Input::SetMousePos(XMFLOAT2 pos)
{
    ms_mousePos = pos;
}

void Input::SetMousePosClient(XMFLOAT2 pos)
{
    ms_mousePosClient = pos;
}

void Input::SetMouseWheelDelta(float delta)
{
    ms_mouseWheelDelta += delta;
}

bool Input::KeyActive(unordered_map<KeyCode::Key, bool> map, KeyCode::Key key)
{
    auto iter = map.find(key);
    if (iter == map.end())
        return false;

    return map.at(key);
}

bool Input::IsKeyDown(KeyCode::Key code)
{
    return KeyActive(ms_currentlyHeldKeys, code) && !KeyActive(ms_keysLastFrame, code);
}

bool Input::IsKey(KeyCode::Key code)
{
    return KeyActive(ms_currentlyHeldKeys, code);
}

bool Input::IsKeyUp(KeyCode::Key code)
{
    return !KeyActive(ms_currentlyHeldKeys, code) && KeyActive(ms_keysLastFrame, code);
}

bool Input::IsShiftHeld()
{
    return (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
}

bool Input::IsCtrlHeld()
{
    return (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
}

bool Input::IsAltHeld()
{
    return (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
}

bool Input::IsMouseLeftDown()
{
    return ms_mouseLeftState && !ms_mouseLeftStateLastFrame;
}

bool Input::IsMouseLeft()
{
    return ms_mouseLeftState;
}

bool Input::IsMouseLeftUp()
{
    return !ms_mouseLeftState && ms_mouseLeftStateLastFrame;
}

bool Input::IsMouseRightDown()
{
    return ms_mouseRightState && !ms_mouseRightStateLastFrame;
}

bool Input::IsMouseRight()
{
    return ms_mouseRightState;
}

bool Input::IsMouseRightUp()
{
    return !ms_mouseRightState && ms_mouseRightStateLastFrame;
}

bool Input::IsMouseMiddleDown()
{
    return ms_mouseMiddleState && !ms_mouseMiddleStateLastFrame;
}

bool Input::IsMouseMiddle()
{
    return ms_mouseMiddleState;
}

bool Input::IsMouseMiddleUp()
{
    return !ms_mouseMiddleState && ms_mouseMiddleStateLastFrame;
}

XMFLOAT2 Input::GetMousePos()
{
    return ms_mousePos;
}

XMFLOAT2 Input::GetMousePosDelta()
{
    return XMFLOAT2(ms_mousePos.x - ms_mousePosLastFrame.x, ms_mousePos.y - ms_mousePosLastFrame.y);
}

XMFLOAT2 Input::GetMousePosClient()
{
    return ms_mousePosClient;
}

XMFLOAT2 Input::GetMousePosClientDelta()
{
    return XMFLOAT2(ms_mousePosClient.x - ms_mousePosClientLastFrame.x, ms_mousePosClient.y - ms_mousePosClientLastFrame.y);
}

float Input::GetMouseWheelDelta()
{
    return ms_mouseWheelDelta;
}

void Input::ProgressFrame()
{
    ms_keysLastFrame = ms_currentlyHeldKeys;

    ms_mouseLeftStateLastFrame = ms_mouseLeftState;
    ms_mouseRightStateLastFrame = ms_mouseRightState;
    ms_mouseMiddleStateLastFrame = ms_mouseMiddleState;

    ms_mousePosLastFrame = ms_mousePos;
    ms_mousePosClientLastFrame = ms_mousePosClient;

    ms_mouseWheelDelta = 0;
}