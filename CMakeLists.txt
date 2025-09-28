cmake_minimum_required(VERSION 4.0)
project(PT)

set(CMAKE_CXX_STANDARD 20)

# Set O0 for debug builds
set(CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG} /Od /Zi")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /Zi")

# Use the Release CRT for all configurations
if(MSVC)
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

add_executable(PT WIN32
        src/main.cpp
        Apps/HelloTriangle/src/HelloTriangle.cpp
        Apps/HelloTriangle/Headers/HelloTriangle.h
        Apps/SpinningCube/src/SpinningCube.cpp
        Apps/SpinningCube/Headers/SpinningCube.h
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
)

file(GLOB THIRD_PARTY_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/ThirdParty/imgui/*.cpp"
        "${CMAKE_SOURCE_DIR}/ThirdParty/imgui/*.h"
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

file(GLOB_RECURSE ASSET_FILES "${CMAKE_SOURCE_DIR}/Assets/*")

add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/Assets.stamp
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:PT>/Assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Assets
        $<TARGET_FILE_DIR:PT>/Assets
        COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_BINARY_DIR}/Assets.stamp
        DEPENDS ${ASSET_FILES}
)

add_custom_target(CopyAssets ALL
        DEPENDS ${CMAKE_BINARY_DIR}/Assets.stamp
)
add_dependencies(CopyAssets PT)

FetchContent_Declare(
        fastgltf
        URL https://github.com/spnda/fastgltf/archive/refs/tags/v0.9.0.zip
)
FetchContent_MakeAvailable(fastgltf)

file(GLOB FASTGLTF_SOURCES CONFIGURE_DEPENDS
        "${fastgltf_SOURCE_DIR}/src/*.cpp"
        "${fastgltf_SOURCE_DIR}/include/fastgltf/*.hpp"
        "${fastgltf_SOURCE_DIR}/deps/simdjson/*"
)
target_sources(PT PRIVATE
        ${FASTGLTF_SOURCES}
)
target_include_directories(PT PRIVATE
        "${fastgltf_SOURCE_DIR}/include"
        "${fastgltf_SOURCE_DIR}/deps/simdjson"
)