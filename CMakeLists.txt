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

add_library(client STATIC
        ${PROJECT_SOURCES}
        ${PROJECT_HEADERS}
)

include_directories(
        Headers
        Headers/client
        Assets/Shaders
        Assets/Shaders/DualIncludes
        Assets/Resources
)

include(cmake/Dependencies.cmake)
include(cmake/Assets.cmake)

target_compile_definitions(client PUBLIC NOMINMAX)
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_CONFIGURATION_TYPES MATCHES "Debug")
    target_compile_definitions(client PUBLIC SOURCE_DIR="${CMAKE_SOURCE_DIR}")
endif()

add_subdirectory(Apps)