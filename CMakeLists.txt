cmake_minimum_required(VERSION 4.0)
project(CherryPip)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set O0 for debug builds
set(CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG} /Od /Zi")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /Zi")

if(MSVC)
    include(cmake/MsvcFlags.cmake)
endif()

include(cmake/Sources.cmake)

add_executable(CherryPip WIN32
        src/main.cpp
        ${PROJECT_SOURCES}
        ${PROJECT_HEADERS}
        Assets/Resources/CherryPip.rc
        Assets/Resources/Resources.h
)

include_directories(
        Headers
        Headers/client
        Assets/Shaders/DualIncludes
        ${CMAKE_SOURCE_DIR}
)

include(cmake/Dependencies.cmake)
include(cmake/Assets.cmake)

target_compile_definitions(CherryPip PRIVATE NOMINMAX)