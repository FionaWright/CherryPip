cmake_minimum_required(VERSION 4.0)
project(PT)

set(CMAKE_CXX_STANDARD 20)

include(fetchContent)

FetchContent_Declare(
        d3d12agility
        URL https://www.nuget.org/api/v2/package/Microsoft.Direct3D.D3D12/1.616.1
)

FetchContent_MakeAvailable(d3d12agility)
set(D3D12_AGILITY_DIR ${d3d12agility_SOURCE_DIR})
include_directories(${D3D12_AGILITY_DIR}/build/native/include)
set(D3D12_LIBS
        ${D3D12_AGILITY_DIR}/build/native/lib/x64/d3d12.lib
        dxgi.lib   # comes from Windows SDK
)

add_executable(PT main.cpp
        HelloTriangle.cpp
        HelloTriangle.h)

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
add_custom_command(TARGET PT POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${D3D12_AGILITY_DIR}/build/native/bin/x64/dxil.dll
        $<TARGET_FILE_DIR:PT>
)
