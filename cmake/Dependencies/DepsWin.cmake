include(${CMAKE_CURRENT_LIST_DIR}/nuget.cmake)

_ww_find_nuget()

# DirectX 12 Agility SDK
message("Setting up DirectX 12 Agility...")
_ww_load_nuget_dependency(${NUGET_EXE} "Microsoft.Direct3D.D3D12" DXA
        ${CMAKE_CURRENT_BINARY_DIR}/nuget)

string(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)$" VERSION_MATCH ${DXA_DIR})

message("Agility version: ${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${CMAKE_MATCH_3}")
set(DXA_VERSION
        ${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${CMAKE_MATCH_3}
        CACHE INTERNAL "")
set(VERSION_MINOR
        ${CMAKE_MATCH_2}
        CACHE INTERNAL "")

set(DXA_HEADERS ${DXA_DIR}/build/native/include)
set(DXA_SRC ${DXA_DIR}/build/native/src)
set(DXA_BIN ${DXA_DIR}/build/native/bin/x64)
set(DXAGILITY_DLL
        ${DXA_BIN}/D3D12Core.dll
        CACHE INTERNAL "")
set(DXAGILITY_DEBUG_DLL
        ${DXA_BIN}/d3d12SDKLayers.dll
        CACHE INTERNAL "")

add_library(DX12AgilityCore MODULE IMPORTED GLOBAL)
set_property(TARGET DX12AgilityCore PROPERTY IMPORTED_LOCATION
        ${DXAGILITY_DLL})

add_library(DX12AgilitySDKLayers MODULE IMPORTED GLOBAL)
set_property(TARGET DX12AgilitySDKLayers PROPERTY IMPORTED_LOCATION
        ${DXAGILITY_DEBUG_DLL})

# Header interface library
add_library(DX12Agility STATIC)
add_library(wis::DX12Agility ALIAS DX12Agility)

target_include_directories(
        DX12Agility SYSTEM BEFORE
        PUBLIC $<BUILD_INTERFACE:${DXA_HEADERS}> $<INSTALL_INTERFACE:include/d3dx12>
        PRIVATE $<BUILD_INTERFACE:${DXA_HEADERS}/d3dx12>)
target_sources(DX12Agility
        PRIVATE ${DXA_SRC}/d3dx12/d3dx12_property_format_table.cpp)

set_target_properties(DX12Agility PROPERTIES
        DX12SDKVER ${VERSION_MINOR}
        DEBUG_POSTFIX d
)

set_property(
        TARGET DX12Agility
        APPEND
        PROPERTY EXPORT_PROPERTIES DX12SDKVER)

# WinPix Event Runtime
message("Setting up WinPix Event Runtime...")
_ww_load_nuget_dependency(${NUGET_EXE} "WinPixEventRuntime" WinPix
        ${CMAKE_CURRENT_BINARY_DIR}/nuget)

add_library(WinPixEventRuntime SHARED IMPORTED GLOBAL)
set_target_properties(WinPixEventRuntime PROPERTIES
        IMPORTED_LOCATION ${WinPix_DIR}/bin/x64/WinPixEventRuntime.dll
        IMPORTED_IMPLIB ${WinPix_DIR}/bin/x64/WinPixEventRuntime.lib
        INTERFACE_INCLUDE_DIRECTORIES ${WinPix_DIR}/include
)

# Some functions

function(wis_export_agility_file)
    set(options)
    set(oneValueArgs PATH)
    set(multiValueArgs)

    cmake_parse_arguments(wis_export_agility_file
            "${options}" "${oneValueArgs}" "${multiValueArgs}"
            ${ARGN})

    get_property(DX12SDKVER TARGET wis::DX12Agility PROPERTY DX12SDKVER)

    set(EXPORT_AGILITY "_declspec(dllexport) const unsigned D3D12SDKVersion = ${DX12SDKVER};
						_declspec(dllexport) const char* D3D12SDKPath = \".\\\\D3D12\\\\\";"
    )
    file(WRITE ${wis_export_agility_file_PATH} "${EXPORT_AGILITY}")
endfunction()

function(wis_install_dx_win32 PROJECT)
    message("Installing DirectX Agility SDK Dependency")
    wis_export_agility_file(PATH "${CMAKE_CURRENT_BINARY_DIR}/exports.c")

    target_sources(${PROJECT} PRIVATE
            ${CMAKE_CURRENT_BINARY_DIR}/exports.c
    )

    get_filename_component(DXAGILITY_DLL_NAME ${DXAGILITY_DLL} NAME)
    add_custom_command(TARGET ${PROJECT} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DXAGILITY_DLL} $<TARGET_FILE_DIR:${PROJECT}>/D3D12/${DXAGILITY_DLL_NAME}
            COMMAND_EXPAND_LISTS
            COMMENT "Copying DX12 Agility Core..."
    )


    get_filename_component(DXAGILITY_DEBUG_DLL_NAME ${DXAGILITY_DEBUG_DLL} NAME)
    add_custom_command(TARGET ${PROJECT} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${DXAGILITY_DEBUG_DLL} $<TARGET_FILE_DIR:${PROJECT}>/D3D12/${DXAGILITY_DEBUG_DLL_NAME}
            COMMAND_EXPAND_LISTS
            COMMENT "Copying DX12 Agility SDKLayers..."
    )
endfunction()