

#include "Apps/SceneStudio/Headers/SceneStudio.h"
#include "System/CherryPip.h"

_Use_decl_annotations_

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR args, int nCmdShow)
{
    SceneStudio pt;
    return CherryPip::Run(pt, hInstance, args, nCmdShow);
}
