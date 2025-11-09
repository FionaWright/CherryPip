

#include "Apps/PathTracer/Headers/PathTracer.h"
#include "System/CherryPip.h"

_Use_decl_annotations_

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR args, int nCmdShow)
{
    PathTracer pt;
    return CherryPip::Run(pt, hInstance, args, nCmdShow);
}
