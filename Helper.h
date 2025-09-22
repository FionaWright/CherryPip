#ifndef H_HELPER_H
#define H_HELPER_H

#include <string>
#include <iostream>
#include <windows.h>
#include <locale>
#include <codecvt>

// D3D12 extension library.
#include "d3dx12.h"
#include "HelloTriangle.h"

using Microsoft::WRL::ComPtr;
#include <dxcapi.h>
#include <fstream>

inline std::string wstringtoString(const std::wstring& wstr)
{
    //setup converter
    using convert_type = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_type, wchar_t> converter;

    //use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
    return converter.to_bytes( wstr );
}

inline std::string HrToString(HRESULT hr)
{
    char s_str[64] = {};
    sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
    return std::string(s_str);
}

class HrException : public std::runtime_error
{
public:
    HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) {}
    HRESULT Error() const { return m_hr; }
private:
    const HRESULT m_hr;
};

inline void DumpDebugMessages(ID3D12Device* device)
{
    ComPtr<ID3D12InfoQueue> infoQueue;
    if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue))))
    {
        const UINT64 messageCount = infoQueue->GetNumStoredMessages();
        for (UINT64 i = 0; i < messageCount; ++i)
        {
            SIZE_T messageLength = 0;
            infoQueue->GetMessage(i, nullptr, &messageLength);
            std::vector<char> bytes(messageLength);
            auto* message = reinterpret_cast<D3D12_MESSAGE*>(bytes.data());
            infoQueue->GetMessage(i, message, &messageLength);
            std::cout << "D3D12: " << message->pDescription << std::endl;
        }
        infoQueue->ClearStoredMessages();
    }
}

inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        DumpDebugMessages(HelloTriangle::s_device);
        std::cout << "ERR: " + HrToString(hr) << std::endl;
        throw HrException(hr);
    }
}

inline void GetAssetsPath(_Out_writes_(pathSize) WCHAR* path, UINT pathSize)
{
    if (path == nullptr)
    {
        throw std::exception();
    }

    DWORD size = GetModuleFileNameW(nullptr, path, pathSize);
    if (size == 0 || size == pathSize)
    {
        // Method failed or path was truncated.
        throw std::exception();
    }

    WCHAR* lastSlash = wcsrchr(path, L'\\');
    if (lastSlash)
    {
        *(lastSlash + 1) = L'\0';
    }
}

inline HRESULT ReadDataFromFile(LPCWSTR filename, byte** data, UINT* size)
{
    using namespace Microsoft::WRL;

#if WINVER >= _WIN32_WINNT_WIN8
    CREATEFILE2_EXTENDED_PARAMETERS extendedParams = {};
    extendedParams.dwSize = sizeof(CREATEFILE2_EXTENDED_PARAMETERS);
    extendedParams.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
    extendedParams.dwFileFlags = FILE_FLAG_SEQUENTIAL_SCAN;
    extendedParams.dwSecurityQosFlags = SECURITY_ANONYMOUS;
    extendedParams.lpSecurityAttributes = nullptr;
    extendedParams.hTemplateFile = nullptr;

    Wrappers::FileHandle file(CreateFile2(filename, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, &extendedParams));
#else
    Wrappers::FileHandle file(CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN | SECURITY_SQOS_PRESENT | SECURITY_ANONYMOUS, nullptr));
#endif
    if (file.Get() == INVALID_HANDLE_VALUE)
    {
        throw std::exception();
    }

    FILE_STANDARD_INFO fileInfo = {};
    if (!GetFileInformationByHandleEx(file.Get(), FileStandardInfo, &fileInfo, sizeof(fileInfo)))
    {
        throw std::exception();
    }

    if (fileInfo.EndOfFile.HighPart != 0)
    {
        throw std::exception();
    }

    *data = reinterpret_cast<byte*>(malloc(fileInfo.EndOfFile.LowPart));
    *size = fileInfo.EndOfFile.LowPart;

    if (!ReadFile(file.Get(), *data, fileInfo.EndOfFile.LowPart, nullptr, nullptr))
    {
        throw std::exception();
    }

    return S_OK;
}

inline HRESULT ReadDataFromDDSFile(LPCWSTR filename, byte** data, UINT* offset, UINT* size)
{
    if (FAILED(ReadDataFromFile(filename, data, size)))
    {
        return E_FAIL;
    }

    // DDS files always start with the same magic number.
    static const UINT DDS_MAGIC = 0x20534444;
    UINT magicNumber = *reinterpret_cast<const UINT*>(*data);
    if (magicNumber != DDS_MAGIC)
    {
        return E_FAIL;
    }

    struct DDS_PIXELFORMAT
    {
        UINT size;
        UINT flags;
        UINT fourCC;
        UINT rgbBitCount;
        UINT rBitMask;
        UINT gBitMask;
        UINT bBitMask;
        UINT aBitMask;
    };

    struct DDS_HEADER
    {
        UINT size;
        UINT flags;
        UINT height;
        UINT width;
        UINT pitchOrLinearSize;
        UINT depth;
        UINT mipMapCount;
        UINT reserved1[11];
        DDS_PIXELFORMAT ddsPixelFormat;
        UINT caps;
        UINT caps2;
        UINT caps3;
        UINT caps4;
        UINT reserved2;
    };

    auto ddsHeader = reinterpret_cast<const DDS_HEADER*>(*data + sizeof(UINT));
    if (ddsHeader->size != sizeof(DDS_HEADER) || ddsHeader->ddsPixelFormat.size != sizeof(DDS_PIXELFORMAT))
    {
        return E_FAIL;
    }

    const ptrdiff_t ddsDataOffset = sizeof(UINT) + sizeof(DDS_HEADER);
    *offset = ddsDataOffset;
    *size = *size - ddsDataOffset;

    return S_OK;
}

// Assign a name to the object to aid with debugging.
#if defined(_DEBUG) || defined(DBG)
inline void SetName(ID3D12Object* pObject, LPCWSTR name)
{
    pObject->SetName(name);
}
inline void SetNameIndexed(ID3D12Object* pObject, LPCWSTR name, UINT index)
{
    WCHAR fullName[50];
    if (swprintf_s(fullName, L"%s[%u]", name, index) > 0)
    {
        pObject->SetName(fullName);
    }
}
#else
inline void SetName(ID3D12Object*, LPCWSTR)
{
}
inline void SetNameIndexed(ID3D12Object*, LPCWSTR, UINT)
{
}
#endif

// Naming helper for ComPtr<T>.
// Assigns the name of the variable as the name of the object.
// The indexed variant will include the index in the name of the object.
#define NAME_D3D12_OBJECT(x) SetName((x).Get(), L#x)
#define NAME_D3D12_OBJECT_INDEXED(x, n) SetNameIndexed((x)[n].Get(), L#x, n)

inline UINT CalculateConstantBufferByteSize(UINT byteSize)
{
    // Constant buffer size is required to be aligned.
    return (byteSize + (D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1)) & ~(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1);
}

#ifdef D3D_COMPILE_STANDARD_FILE_INCLUDE
inline Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(
    const std::wstring& filename,
    const D3D_SHADER_MACRO* defines,
    const std::string& entrypoint,
    const std::string& target)
{
    UINT compileFlags = 0;
#if defined(_DEBUG) || defined(DBG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    HRESULT hr;

    Microsoft::WRL::ComPtr<ID3DBlob> byteCode = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errors;
    hr = D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entrypoint.c_str(), target.c_str(), compileFlags, 0, &byteCode, &errors);

    if (errors != nullptr)
    {
        OutputDebugStringA((char*)errors->GetBufferPointer());
    }
    ThrowIfFailed(hr);

    return byteCode;
}
#endif

// Resets all elements in a ComPtr array.
template<class T>
void ResetComPtrArray(T* comPtrArray)
{
    for (auto &i : *comPtrArray)
    {
        i.Reset();
    }
}


// Resets all elements in a unique_ptr array.
template<class T>
void ResetUniquePtrArray(T* uniquePtrArray)
{
    for (auto &i : *uniquePtrArray)
    {
        i.reset();
    }
}

inline std::vector<uint8_t> ReadFileToByteVector(const std::wstring& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) throw std::runtime_error("Failed to open file");
    auto size = static_cast<size_t>(file.tellg());
    std::vector<uint8_t> data(size);
    file.seekg(0);
    file.read(reinterpret_cast<char*>(data.data()), size);
    return data;
}

inline ComPtr<IDxcBlob> CompileShaderDXC(
    const std::wstring& filePath,
    LPCWSTR entryPoint,
    LPCWSTR targetProfile,
    UINT compileFlags)
{
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
    }

    // Create DXC objects
    ComPtr<IDxcCompiler3> compiler;
    ComPtr<IDxcLibrary> library;
    ComPtr<IDxcIncludeHandler> includeHandler;
    ComPtr<IDxcUtils> utils;
    try {
        ThrowIfFailed(DxcCreateInstanceFn(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler)));
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    } catch (...) {
        OutputDebugStringA("Unknown exception in DxcCreateInstance");
    }

    ThrowIfFailed(DxcCreateInstanceFn(CLSID_DxcLibrary, IID_PPV_ARGS(&library)));
    ThrowIfFailed(DxcCreateInstanceFn(CLSID_DxcUtils, IID_PPV_ARGS(&utils)));
    ThrowIfFailed(library->CreateIncludeHandler(&includeHandler));

    auto shaderBytes = ReadFileToByteVector(filePath);

    DxcBuffer buffer;
    buffer.Ptr = shaderBytes.data();
    buffer.Size = shaderBytes.size();
    buffer.Encoding = DXC_CP_UTF8; // or DXC_CP_ACP if ASCII

    // Compile vertex shader
    ComPtr<IDxcResult> result;
    const wchar_t* args[] = { L"-E", entryPoint, L"-T", targetProfile };
    if (FAILED(compiler->Compile(&buffer, args, _countof(args), includeHandler.Get(), IID_PPV_ARGS(&result)))) {
        std::cerr << "Vertex shader compile failed\n";
        return nullptr;
    }

    // Get compiled blob
    ComPtr<IDxcBlobUtf8> errors;
    if (FAILED(result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr))) {
        std::cerr << "Failed to get shader errors\n";
        return nullptr;
    }
    if (errors && errors->GetStringLength() > 0) {
        std::cout << "Shader compile warnings/errors:\n" << errors->GetStringPointer() << "\n";
    }

    ComPtr<IDxcBlob> vertexShaderBlob;
    if (FAILED(result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&vertexShaderBlob), nullptr))) {
        std::cerr << "Failed to get compiled shader\n";
        return nullptr;
    }

    return vertexShaderBlob;
}

#endif