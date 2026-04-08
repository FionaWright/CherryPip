# Source files
set(PROJECT_SOURCES
    "src/MathUtils.cpp"
    "src/HWI/BLAS.cpp"
    "src/HWI/D12Resource.cpp"
    "src/HWI/D3D.cpp"
    "src/HWI/Heap.cpp"
    "src/HWI/Material.cpp"
    "src/HWI/Model.cpp"
    "src/HWI/RootSig.cpp"
    "src/HWI/Shader.cpp"
    "src/HWI/Texture.cpp"
    "src/HWI/TLAS.cpp"
    "src/Render/Camera.cpp"
    "src/Render/CameraController.cpp"
    "src/Render/DeferredContext.cpp"
    "src/Render/DenoisingManager.cpp"
    "src/Render/EnvMap.cpp"
    "src/Render/Object.cpp"
    "src/Render/PathTracingContext.cpp"
    "src/Render/RasterContext.cpp"
    "src/Render/Scene.cpp"
    "src/Render/Skybox.cpp"
    "src/Render/TextureRTV.cpp"
    "src/Render/Transform.cpp"
    "src/System/App.cpp"
    "src/System/CherryPip.cpp"
    "src/System/Config.cpp"
    "src/System/Engine.cpp"
    "src/System/Engine_GUI.cpp"
    "src/System/FileHelper.cpp"
    "src/System/Gui.cpp"
    "src/System/HighResolutionClock.cpp"
    "src/System/ImGuiUtils.cpp"
    "src/System/Input.cpp"
    "src/System/ModelLoaderGLTF.cpp"
    "src/System/ResourceSharer.cpp"
    "src/System/TextureLoader.cpp"
    "src/System/Win32App.cpp"
)

# Header files
set(PROJECT_HEADERS
    "include/client/Helper.h"
    "include/client/MathUtils.h"
    "include/client/HWI/BLAS.h"
    "include/client/HWI/D12Resource.h"
    "include/client/HWI/D3D.h"
    "include/client/HWI/Heap.h"
    "include/client/HWI/Material.h"
    "include/client/HWI/Model.h"
    "include/client/HWI/RootSig.h"
    "include/client/HWI/Shader.h"
    "include/client/HWI/Texture.h"
    "include/client/HWI/TLAS.h"
    "include/client/Render/Camera.h"
    "include/client/Render/CameraController.h"
    "include/client/Render/DeferredContext.h"
    "include/client/Render/DenoisingManager.h"
    "include/client/Render/EnvMap.h"
    "include/client/Render/Object.h"
    "include/client/Render/PathTracingContext.h"
    "include/client/Render/RasterContext.h"
    "include/client/Render/Scene.h"
    "include/client/Render/Skybox.h"
    "include/client/Render/TextureRTV.h"
    "include/client/Render/Transform.h"
    "include/client/System/App.h"
    "include/client/System/CherryPip.h"
    "include/client/System/Config.h"
    "include/client/System/FileHelper.h"
    "include/client/System/Gui.h"
    "include/client/System/HighResolutionClock.h"
    "include/client/System/ImGuiUtils.h"
    "include/client/System/Input.h"
    "include/client/System/Keycodes.h"
    "include/client/System/ResourceSharer.h"
    "include/client/System/TextureLoader.h"
    "include/client/System/Win32App.h"
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_CONFIGURATION_TYPES MATCHES "Debug")
    set(PROJECT_DEBUG_SOURCES
        "src/Debug/DebugLine.cpp"
        "src/Debug/GPUEventScoped.cpp"
        "src/Debug/HotReloader.cpp"
        "src/Debug/PathVisualizer.cpp"
        "src/Debug/PythonExecutor.cpp"
        "src/Debug/Profiler.cpp"
        "src/Debug/ReadbackBuffer.cpp"
        "src/Debug/RmseTester.cpp"
    )
    set(PROJECT_DEBUG_HEADERS
        "include/client/Debug/DebugOutputRedirector.h"
        "include/client/Debug/DebugLine.h"
        "include/client/Debug/GPUEventScoped.h"
        "include/client/Debug/HotReloader.h"
        "include/client/Debug/PathVisualizer.h"
        "include/client/Debug/PythonExecutor.h"
        "include/client/Debug/Profiler.h"
        "include/client/Debug/ReadbackBuffer.h"
        "include/client/Debug/RmseTester.h"
    )
endif()

add_library(client STATIC
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
    ${PROJECT_DEBUG_SOURCES}
    ${PROJECT_DEBUG_HEADERS}
)
target_precompile_headers(client PUBLIC
    "$<$<COMPILE_LANGUAGE:CXX>:System/pch.h>"
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
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/client"
    "${CMAKE_SOURCE_DIR}/assets/Shaders/DualIncludes"
    "${CMAKE_SOURCE_DIR}/assets/Shaders"
    "${CMAKE_SOURCE_DIR}/assets/Resources"
)
target_compile_definitions(client PUBLIC NOMINMAX)
set_target_properties(client PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

include(../cmake/Assets.cmake)
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_CONFIGURATION_TYPES MATCHES "Debug")
    target_compile_definitions(client PUBLIC SOURCE_DIR="${CMAKE_SOURCE_DIR}")
endif()