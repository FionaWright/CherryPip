cmake_minimum_required(VERSION 4.0)
project(PT)

set(CMAKE_CXX_STANDARD 20)

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

add_executable(PT WIN32 main.cpp
        HelloTriangle.cpp
        HelloTriangle.h
        Win32App.cpp
        Win32App.h
        Helper.h)

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