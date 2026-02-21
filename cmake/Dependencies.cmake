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

target_link_libraries(client PRIVATE ${D3D12_LIBS})

add_custom_command(TARGET client POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${D3D12_AGILITY_DIR}/build/native/bin/x64/D3D12Core.dll
        $<TARGET_FILE_DIR:client>/D3D12/D3D12Core.dll
)
add_custom_command(TARGET client POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${D3D12_AGILITY_DIR}/build/native/bin/x64/D3D12SDKLayers.dll
        $<TARGET_FILE_DIR:client>/D3D12/D3D12SDKLayers.dll
)

FetchContent_Declare(
        dxc
        URL https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.8.2505.1/dxc_2025_07_14.zip
)
FetchContent_MakeAvailable(dxc)

set(DXC_DIR ${dxc_SOURCE_DIR})

target_link_libraries(client PRIVATE ${DXC_DIR}/lib/x64/dxil.lib)
target_link_libraries(client PRIVATE ${DXC_DIR}/lib/x64/dxcompiler.lib)
include_directories(${DXC_DIR}/inc)

add_custom_command(TARGET client POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${DXC_DIR}/bin/x64/dxcompiler.dll
        $<TARGET_FILE_DIR:client>
)

add_custom_command(TARGET client POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${DXC_DIR}/bin/x64/dxil.dll
        $<TARGET_FILE_DIR:client>
)

file(GLOB THIRD_PARTY_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/ThirdParty/imgui/*.cpp"
        "${CMAKE_SOURCE_DIR}/ThirdParty/imgui/*.h"
        "${CMAKE_SOURCE_DIR}/ThirdParty/spng/*"
        "${CMAKE_SOURCE_DIR}/ThirdParty/zlib/*"
        "${CMAKE_SOURCE_DIR}/ThirdParty/tinyddsloader/tinyddsloader.h"
        "${CMAKE_SOURCE_DIR}/ThirdParty/WinPixEventRuntime.1.0.240308001/Include/WinPixEventRuntime/*"
)
target_sources(client PUBLIC
        ${THIRD_PARTY_SOURCES}
        "${CMAKE_SOURCE_DIR}/ThirdParty/imgui/backends/imgui_impl_dx12.h"
        "${CMAKE_SOURCE_DIR}/ThirdParty/imgui/backends/imgui_impl_dx12.cpp"
        "${CMAKE_SOURCE_DIR}/ThirdParty/imgui/backends/imgui_impl_win32.cpp"
        "${CMAKE_SOURCE_DIR}/ThirdParty/imgui/backends/imgui_impl_win32.h"
)
target_include_directories(client PUBLIC
        "${CMAKE_SOURCE_DIR}/ThirdParty"
        "${CMAKE_SOURCE_DIR}/ThirdParty/imgui"
        "${CMAKE_SOURCE_DIR}/ThirdParty/spng"
        "${CMAKE_SOURCE_DIR}/ThirdParty/zlib"
        "${CMAKE_SOURCE_DIR}/ThirdParty/tinyddsloader"
        "${CMAKE_SOURCE_DIR}/ThirdParty/WinPixEventRuntime.1.0.240308001/Include/"
)

target_compile_definitions(client PUBLIC TINYDDSLOADER_IMPLEMENTATION)

add_custom_command(TARGET client POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_SOURCE_DIR}/ThirdParty/WinPixEventRuntime.1.0.240308001/bin/x64/WinPixEventRuntime.dll"
        $<TARGET_FILE_DIR:client>
)
target_link_libraries(client PUBLIC "${CMAKE_SOURCE_DIR}/ThirdParty/WinPixEventRuntime.1.0.240308001/bin/x64/WinPixEventRuntime.lib")

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
target_link_libraries(client PUBLIC fastgltf)

target_link_libraries(client PUBLIC dxguid uuid)