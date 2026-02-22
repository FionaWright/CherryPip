include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
set(CPM_DONT_UPDATE_MODULE_PATH ON)
set(GET_CPM_FILE "${CMAKE_CURRENT_LIST_DIR}/Dependencies/get_cpm.cmake")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Set CPM source cache
if (NOT CPM_SOURCE_CACHE)
    set(CPM_SOURCE_CACHE "${CMAKE_CURRENT_BINARY_DIR}/_deps_cache")
endif ()

# Get CPM
if (NOT EXISTS ${GET_CPM_FILE})
    file(DOWNLOAD
            https://github.com/cpm-cmake/CPM.cmake/releases/latest/download/get_cpm.cmake
            "${GET_CPM_FILE}"
    )
endif ()
include(${GET_CPM_FILE})

if (WIN32)
    include(${CMAKE_CURRENT_LIST_DIR}/Dependencies/DepsWin.cmake)
endif()

# Download DXC if needed
include(${CMAKE_CURRENT_LIST_DIR}/Dependencies/DXC.cmake)

# CPM dependencies

# ImGui
CPMAddPackage(
    NAME imgui
    GITHUB_REPOSITORY ocornut/imgui
    GIT_TAG v1.92.6
    DOWLOAD_ONLY TRUE
)

# Manually make ImGui available as a target
add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp

    # Backends for Win32 and DirectX 12
    ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx12.cpp
)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_WIN32_DISABLE_GAMEPAD)

# fastgltf
CPMAddPackage( 
    NAME fastgltf
    GITHUB_REPOSITORY spnda/fastgltf
    GIT_TAG v0.9.0
)

# Zlib
CPMADDPACKAGE(
    NAME zlib
    GITHUB_REPOSITORY madler/zlib
    GIT_TAG v1.3.2

    OPTIONS
    "ZLIB_BUILD_SHARED OFF"
    "ZLIB_BUILD_TESTING OFF"
)


# TODO: Replace with DirectXTex

# SPNG
add_library(spng STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/spng/spng.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/spng/spng.h
)
target_include_directories(spng 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/spng
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty
)
target_compile_definitions(spng PUBLIC SPNG_STATIC)
target_link_libraries(spng PUBLIC ZLIB::ZLIBSTATIC)

# tinyddsloader
add_library(tinyddsloader STATIC)

# generate tinyddsloader.cpp as simple #include "tinyddsloader.h"
target_compile_definitions(tinyddsloader PRIVATE TINYDDSLOADER_IMPLEMENTATION)
target_include_directories(tinyddsloader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/tinyddsloader)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/tinyddsloader.cpp "#include \"tinyddsloader.h\"\n")

target_sources(tinyddsloader PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/tinyddsloader.cpp)
