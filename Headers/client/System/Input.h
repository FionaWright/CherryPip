//
// Created by fionaw on 25/09/2025.
//

#ifndef PT_INPUT_H
#define PT_INPUT_H

#include "Keycodes.h"
#include "DirectXMath.h"

#include <unordered_map>
using std::unordered_map;

using namespace DirectX;

class Input
{
public:
    static void AddKey(KeyCode::Key keyState);
    static void RemoveKey(KeyCode::Key keyState);
    static void SetMouseLeftState(bool state);
    static void SetMouseRightState(bool state);
    static void SetMouseMiddleState(bool state);
    static void SetMousePos(XMFLOAT2 pos);
    static void SetMousePosClient(XMFLOAT2 pos);
    static void SetMouseWheelDelta(float delta);

    static bool KeyActive(unordered_map<KeyCode::Key, bool> map, KeyCode::Key key);

    static bool IsKeyDown(KeyCode::Key code);
    static bool IsKey(KeyCode::Key code);
    static bool IsKeyUp(KeyCode::Key code);

    static bool IsShiftHeld();
    static bool IsCtrlHeld();
    static bool IsAltHeld();

    static bool IsMouseLeftDown();
    static bool IsMouseLeft();
    static bool IsMouseLeftUp();

    static bool IsMouseRightDown();
    static bool IsMouseRight();
    static bool IsMouseRightUp();

    static bool IsMouseMiddleDown();
    static bool IsMouseMiddle();
    static bool IsMouseMiddleUp();

    static XMFLOAT2 GetMousePos();
    static XMFLOAT2 GetMousePosDelta();
    static XMFLOAT2 GetMousePosClient();
    static XMFLOAT2 GetMousePosClientDelta();

    static float GetMouseWheelDelta();

    static void ProgressFrame();

private:
    static unordered_map<KeyCode::Key, bool> ms_currentlyHeldKeys;
    static unordered_map<KeyCode::Key, bool> ms_keysLastFrame;

    static XMFLOAT2 ms_mousePos;
    static XMFLOAT2 ms_mousePosLastFrame;

    static XMFLOAT2 ms_mousePosClient;
    static XMFLOAT2 ms_mousePosClientLastFrame;

    static float ms_mouseWheelDelta;

    static bool ms_mouseLeftState;
    static bool ms_mouseRightState;
    static bool ms_mouseMiddleState;

    static bool ms_mouseLeftStateLastFrame;
    static bool ms_mouseRightStateLastFrame;
    static bool ms_mouseMiddleStateLastFrame;
};


#endif //PT_INPUT_H