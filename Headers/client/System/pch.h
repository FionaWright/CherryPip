#ifndef H_PCH_H
#define H_PCH_H

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// Windows Runtime Library. Needed for ComPtr<> template class.
#include <wrl.h>
#include <wrl/client.h>

// DirectX 12 specific headers.
#include <d3d12.h>
#include <dxgi1_5.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <dxcapi.h>
#include <DirectXMath.h>
#include <dxgiformat.h>

// D3D12 extension library.
#include "d3dx12.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

#include <fstream>
#include <queue>
#include <memory>
#include <vector>
#include <string>
#include <cstdint>
#include <unordered_map>
#include <chrono>
#include <stack>
#include <mutex>
#include <variant>
#include <shellapi.h>
#include <fstream>
#include <locale>
#include <codecvt>
#include <filesystem>
#include <cassert>
#include <iostream>
#include <span>
#include <cmath>
#include <sstream>
#include <direct.h>
#include <ostream>
#include <algorithm>

#endif