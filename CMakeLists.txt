cmake_minimum_required(VERSION 4.0)
project(CherryPip)

if(MSVC)
    include(cmake/MsvcFlags.cmake)
endif()

include(cmake/Dependencies.cmake)
include(cmake/Sources.cmake)

add_library(client STATIC
        ${PROJECT_SOURCES}
        ${PROJECT_HEADERS}
)
target_precompile_headers(client PRIVATE
        "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_SOURCE_DIR}/Headers/client/System/pch.h>"
)
target_link_libraries(client 
    PUBLIC
        DX12Agility
        d3d12
        dxgi
        dxguid
        imgui
        fastgltf

        spng
        tinyddsloader

        WinPixEventRuntime
)
target_include_directories(client PUBLIC
    "${CMAKE_SOURCE_DIR}/Headers"
    "${CMAKE_SOURCE_DIR}/Headers/client"
    "${CMAKE_SOURCE_DIR}/Assets/Shaders/DualIncludes"
    "${CMAKE_SOURCE_DIR}/Assets/Shaders"
    "${CMAKE_SOURCE_DIR}/Assets/Resources"
)
target_compile_definitions(client PUBLIC NOMINMAX)
set_target_properties(client PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

include(cmake/Assets.cmake)

if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_CONFIGURATION_TYPES MATCHES "Debug")
    target_compile_definitions(client PUBLIC SOURCE_DIR="${CMAKE_SOURCE_DIR}")
endif()

add_subdirectory(Apps)