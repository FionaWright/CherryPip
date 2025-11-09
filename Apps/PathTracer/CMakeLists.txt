set(PROJECT_SOURCES
        src/PathTracer.cpp
        src/PathTracer_GUI.cpp
)

set(PROJECT_HEADERS
        Headers/PathTracer.h
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_CONFIGURATION_TYPES MATCHES "Debug")
    set(PROJECT_SOURCES
            ${PROJECT_SOURCES}
            src/ReadbackManager.cpp
            src/FurnaceTest.cpp
    )
    set(PROJECT_HEADERS
            ${PROJECT_HEADERS}
            Headers/ReadbackManager.h
            Headers/FurnaceTest.h
    )
endif()

add_executable(PathTracer WIN32
        src/main.cpp
        ${PROJECT_SOURCES}
        ${PROJECT_HEADERS}
        ${CHERRYPIP_ASSETS_DIR}/Resources/CherryPip.rc
        ${CHERRYPIP_ASSETS_DIR}/Resources/Resources.h
)

target_include_directories(PathTracer PRIVATE
        "${CHERRYPIP_ASSETS_DIR}/ThirdParty"
        "${CHERRYPIP_ASSETS_DIR}/ThirdParty/imgui"
        "${CHERRYPIP_ASSETS_DIR}/ThirdParty/spng"
        "${CHERRYPIP_ASSETS_DIR}/ThirdParty/zlib"
        "${CHERRYPIP_ASSETS_DIR}/ThirdParty/WinPixEventRuntime.1.0.240308001/Include/"
)

target_link_libraries(PathTracer PRIVATE client)

add_custom_command(TARGET PathTracer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/WinPixEventRuntime.dll"
        $<TARGET_FILE_DIR:PathTracer>
)

add_custom_command(TARGET PathTracer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/dxcompiler.dll"
        $<TARGET_FILE_DIR:PathTracer>
)

add_custom_command(TARGET PathTracer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/dxil.dll"
        $<TARGET_FILE_DIR:PathTracer>
)
