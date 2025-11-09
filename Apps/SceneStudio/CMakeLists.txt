set(PROJECT_SOURCES
        src/SceneStudio.cpp
        src/SceneStudio_GUI.cpp
)

set(PROJECT_HEADERS
        Headers/SceneStudio.h
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

add_executable(SceneStudio WIN32
        src/main.cpp
        ${PROJECT_SOURCES}
        ${PROJECT_HEADERS}
        ${CHERRYPIP_ASSETS_DIR}/Resources/CherryPip.rc
        ${CHERRYPIP_ASSETS_DIR}/Resources/Resources.h
)

target_include_directories(SceneStudio PRIVATE
        "${CHERRYPIP_ASSETS_DIR}/ThirdParty"
        "${CHERRYPIP_ASSETS_DIR}/ThirdParty/imgui"
        "${CHERRYPIP_ASSETS_DIR}/ThirdParty/spng"
        "${CHERRYPIP_ASSETS_DIR}/ThirdParty/zlib"
        "${CHERRYPIP_ASSETS_DIR}/ThirdParty/WinPixEventRuntime.1.0.240308001/Include/"
)

target_link_libraries(SceneStudio PRIVATE client)

add_custom_command(TARGET SceneStudio POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/WinPixEventRuntime.dll"
        $<TARGET_FILE_DIR:SceneStudio>
)

add_custom_command(TARGET SceneStudio POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/dxcompiler.dll"
        $<TARGET_FILE_DIR:SceneStudio>
)

add_custom_command(TARGET SceneStudio POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/dxil.dll"
        $<TARGET_FILE_DIR:SceneStudio>
)
