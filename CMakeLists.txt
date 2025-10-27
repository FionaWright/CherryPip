cmake_minimum_required(VERSION 4.0)
project(PT)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set O0 for debug builds
set(CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG} /Od /Zi")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /Zi")

# Use the Release CRT for all configurations
if(MSVC)
    add_compile_options(/permissive- /Zc:preprocessor /Zc:__cplusplus)
    foreach(flag_var
            CMAKE_C_FLAGS_DEBUG
            CMAKE_CXX_FLAGS_DEBUG)
        string(REPLACE "/MDd" "/MD" ${flag_var} "${${flag_var}}")
        string(REPLACE "/MTd" "/MT" ${flag_var} "${${flag_var}}")
    endforeach()
endif()

include(fetchContent)

FetchContent_Declare(
        d3d12agility
        URL https://www.nuget.org/api/v2/package/Microsoft.Direct3D.D3D12/1.616.1
)

FetchContent_MakeAvailable(d3d12agility)
set(D3D12_AGILITY_DIR ${d3d12agility_SOURCE_DIR})
include_directories(${D3D12_AGILITY_DIR}/build/native/include)
include_directories(${D3D12_AGILITY_DIR}/build/native/include/d3dx12)
set(D3D12_LIBS
        d3d12.lib
        dxgi.lib   # comes from Windows SDK
)

# TODO: Make this automatic (?)
add_executable(PT WIN32
        src/main.cpp
        Apps/RasterViewer/Headers/RasterViewer.h
        Apps/RasterViewer/src/RasterViewer.cpp
        Apps/PathTracer/Headers/PathTracer.h
        Apps/PathTracer/src/PathTracer.cpp

        src/client/System/Win32App.cpp
        Headers/client/System/Win32App.h
        Headers/Helper.h
        src/client/HWI/D3D.cpp
        Headers/client/HWI/D3D.h
        src/client/System/FileHelper.cpp
        Headers/client/System/FileHelper.h
        src/client/HWI/Shader.cpp
        Headers/client/HWI/Shader.h
        src/client/Render/Camera.cpp
        Headers/client/Render/Camera.h
        Apps/App.cpp
        Apps/App.h
        src/client/Render/Transform.cpp
        Headers/client/Render/Transform.h
        src/client/Render/CameraController.cpp
        Headers/client/Render/CameraController.h
        src/client/System/Input.cpp
        Headers/client/System/Input.h
        Headers/client/System/Keycodes.h
        src/client/HWI/RootSig.cpp
        Headers/client/HWI/RootSig.h
        Headers/client/System/DebugOutputRedirector.h
        src/client/System/Gui.cpp
        Headers/client/System/Gui.h
        src/client/System/Config.cpp
        Headers/client/System/Config.h
        src/client/HWI/Model.cpp
        Headers/client/HWI/Model.h
        src/client/System/ModelLoaderGLTF.cpp
        src/MathUtils.cpp
        Headers/MathUtils.h
        src/client/HWI/Texture.cpp
        Headers/client/HWI/Texture.h
        src/client/HWI/D12Resource.cpp
        Headers/client/HWI/D12Resource.h
        src/client/System/TextureLoader.cpp
        Headers/client/System/TextureLoader.h
        src/client/HWI/Material.cpp
        Headers/client/HWI/Material.h
        src/client/Render/Object.cpp
        Headers/client/Render/Object.h
        src/client/HWI/Heap.cpp
        Headers/client/HWI/Heap.h
        src/client/System/HotReloader.cpp
        Headers/client/System/HotReloader.h
        src/client/System/HighResolutionClock.cpp
        Headers/client/System/HighResolutionClock.h
        src/client/System/HighResolutionClock.cpp
        src/client/System/Engine.cpp
        Headers/client/System/Engine.h
        src/client/HWI/BLAS.cpp
        src/client/HWI/BLAS.h
        src/client/HWI/TLAS.cpp
        src/client/HWI/TLAS.h
)

file(GLOB THIRD_PARTY_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/ThirdParty/imgui/*.cpp"
        "${CMAKE_SOURCE_DIR}/ThirdParty/imgui/*.h"
        "${CMAKE_SOURCE_DIR}/ThirdParty/spng/*"
        "${CMAKE_SOURCE_DIR}/ThirdParty/zlib/*"
)
target_sources(PT PRIVATE
        ${THIRD_PARTY_SOURCES}
        "${CMAKE_SOURCE_DIR}/ThirdParty/imgui/backends/imgui_impl_dx12.h"
        "${CMAKE_SOURCE_DIR}/ThirdParty/imgui/backends/imgui_impl_dx12.cpp"
        "${CMAKE_SOURCE_DIR}/ThirdParty/imgui/backends/imgui_impl_win32.cpp"
        "${CMAKE_SOURCE_DIR}/ThirdParty/imgui/backends/imgui_impl_win32.h"
)
target_include_directories(PT PRIVATE
        "${CMAKE_SOURCE_DIR}/ThirdParty"
        "${CMAKE_SOURCE_DIR}/ThirdParty/imgui"
        "${CMAKE_SOURCE_DIR}/ThirdParty/spng"
        "${CMAKE_SOURCE_DIR}/ThirdParty/zlib"
)

include_directories(Headers)
include_directories(Headers/client)
include_directories(${CMAKE_SOURCE_DIR})

target_link_libraries(PT PRIVATE ${D3D12_LIBS})

add_custom_command(TARGET PT POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${D3D12_AGILITY_DIR}/build/native/bin/x64/D3D12Core.dll
        $<TARGET_FILE_DIR:PT>
)
add_custom_command(TARGET PT POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${D3D12_AGILITY_DIR}/build/native/bin/x64/D3D12SDKLayers.dll
        $<TARGET_FILE_DIR:PT>
)

FetchContent_Declare(
        dxc
        URL https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.8.2505.1/dxc_2025_07_14.zip
)
FetchContent_MakeAvailable(dxc)

set(DXC_DIR ${dxc_SOURCE_DIR})

target_link_libraries(PT PRIVATE ${DXC_DIR}/lib/x64/dxil.lib)
target_link_libraries(PT PRIVATE ${DXC_DIR}/lib/x64/dxcompiler.lib)
include_directories(${DXC_DIR}/inc)

add_custom_command(TARGET PT POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${DXC_DIR}/bin/x64/dxcompiler.dll
        $<TARGET_FILE_DIR:PT>
)

add_custom_command(TARGET PT POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${DXC_DIR}/bin/x64/dxil.dll
        $<TARGET_FILE_DIR:PT>
)

if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_CONFIGURATION_TYPES MATCHES "Release")
    add_custom_target(CopyAssets ALL
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/Assets"
            "${CMAKE_BINARY_DIR}/Assets"
            COMMENT "Copying assets for Release"
    )
    add_dependencies(PT CopyAssets)
    target_compile_definitions(PT PRIVATE
            ASSETS_SOURCE_DIR=L"${CMAKE_BINARY_DIR}/Assets"
    )
else()
    target_compile_definitions(PT PRIVATE
            ASSETS_SOURCE_DIR=L"${CMAKE_SOURCE_DIR}/Assets"
    )
endif()

target_compile_definitions(PT PRIVATE NOMINMAX)

set(FASTGLTF_USE_CUSTOM_SMALLVECTOR OFF CACHE BOOL "" FORCE)
set(FASTGLTF_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(FASTGLTF_ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)
set(FASTGLTF_ENABLE_DOCS OFF CACHE BOOL "" FORCE)
set(FASTGLTF_ENABLE_GLTF_RS OFF CACHE BOOL "" FORCE)
set(FASTGLTF_ENABLE_ASSIMP OFF CACHE BOOL "" FORCE)
set(FASTGLTF_ENABLE_DEPRECATED_EXT ON CACHE BOOL "" FORCE)
set(FASTGLTF_DISABLE_CUSTOM_MEMORY_POOL OFF CACHE BOOL "" FORCE)
set(FASTGLTF_USE_64BIT_FLOAT OFF CACHE BOOL "" FORCE)
set(FASTGLTF_COMPILE_AS_CPP20 OFF CACHE BOOL "" FORCE)
set(FASTGLTF_ENABLE_CPP_MODULES OFF CACHE BOOL "" FORCE)
set(FASTGLTF_USE_STD_MODULE OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
        fastgltf
        URL https://github.com/spnda/fastgltf/archive/refs/tags/v0.9.0.zip
)
FetchContent_MakeAvailable(fastgltf)
target_link_libraries(PT PRIVATE fastgltf)

find_library(DXGUID_LIB dxguid.lib PATHS [HINTS ENV PROGRAMFILES ENV PROGRAMFILES(X86)])
find_library(UUID_LIB uuid.lib PATHS [HINTS ENV PROGRAMFILES ENV PROGRAMFILES(X86)])

target_link_libraries(PT PRIVATE ${DXGUID_LIB} ${UUID_LIB})