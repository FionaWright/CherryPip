set(PROJECT_SOURCES
        src/SceneStudio.cpp
        src/SceneStudio_GUI.cpp
        src/SceneStudio_Scenes.cpp
        src/PathTracer.cpp
        src/PathTracer_GUI.cpp
)

set(PROJECT_HEADERS
        include/SceneStudio.h
        include/PathTracer.h
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_CONFIGURATION_TYPES MATCHES "Debug")
    set(PROJECT_SOURCES
        ${PROJECT_SOURCES}
        src/ReadbackManager.cpp
    )
    set(PROJECT_HEADERS
        ${PROJECT_HEADERS}
        include/ReadbackManager.h
    )
endif()

add_executable(SceneStudio WIN32
        src/main.cpp
        ${PROJECT_SOURCES}
        ${PROJECT_HEADERS}
        ${CHERRYPIP_ASSETS_DIR}/Resources/CherryPip.rc
        ${CHERRYPIP_ASSETS_DIR}/Resources/Resources.h
)

target_link_libraries(SceneStudio PRIVATE client)
set_target_properties(SceneStudio PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_include_directories(SceneStudio PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# copy .dlls to output directory
add_custom_command(TARGET SceneStudio POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:WinPixEventRuntime>
        $<TARGET_FILE_DIR:SceneStudio>
)
add_custom_command(TARGET SceneStudio POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${DXC_DLLS}
        $<TARGET_FILE_DIR:SceneStudio>
)

wis_install_dx_win32(SceneStudio)