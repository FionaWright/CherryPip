
set(TEXCONV_EXE ${CMAKE_SOURCE_DIR}/ThirdParty/texconv/texconv.exe)

# Directories containing textures
set(TEXTURE_DIRS
    "${CMAKE_SOURCE_DIR}/Assets/Textures"
    "${CMAKE_SOURCE_DIR}/Assets/Models"
)

set(GENERATED_DDS_FILES "")

foreach(DIR ${TEXTURE_DIRS})
    file(GLOB_RECURSE PNG_FILES "${DIR}/*.png")

    foreach(PNG ${PNG_FILES})
        # Compute output path in the build directory
        file(RELATIVE_PATH REL_PATH "${CMAKE_SOURCE_DIR}" "${PNG}")
        string(REPLACE ".png" ".dds" DDS_REL "${REL_PATH}")
        set(DDS_OUT "${CMAKE_BINARY_DIR}/${DDS_REL}")

        # Ensure output directory exists
        get_filename_component(DDS_DIR "${DDS_OUT}" DIRECTORY)
        file(MAKE_DIRECTORY "${DDS_DIR}")

        add_custom_command(
                OUTPUT "${DDS_OUT}"
                COMMAND "${TEXCONV_EXE}" -ft dds -f BC7_UNORM -o "${DDS_DIR}" "${PNG}"
                DEPENDS "${PNG}"
                COMMENT "Converting ${PNG} → ${DDS_OUT}"
                VERBATIM
        )

        list(APPEND GENERATED_DDS_FILES "${DDS_OUT}")
    endforeach()
endforeach()

add_custom_target(GenerateDDS ALL
        DEPENDS ${GENERATED_DDS_FILES}
)

# All files under Assets/Models
file(GLOB_RECURSE MODEL_FILES
        "${CMAKE_SOURCE_DIR}/Assets/Models/*"
        "${CMAKE_SOURCE_DIR}/Assets/Resources/*"
)

set(FILES_TO_COPY "")

foreach(f ${MODEL_FILES})
    # Skip PNGs
    if(f MATCHES "\\.png$")
        continue()
    endif()

    list(APPEND FILES_TO_COPY "${f}")
endforeach()

set(COPIED_OUTPUTS "")

foreach(src ${FILES_TO_COPY})
    # Compute relative path under Assets/Models
    file(RELATIVE_PATH rel "${CMAKE_SOURCE_DIR}/Assets/Models" "${src}")
    set(dst "${CMAKE_BINARY_DIR}/Assets/Models/${rel}")

    # Ensure destination directory exists
    get_filename_component(dst_dir "${dst}" DIRECTORY)
    file(MAKE_DIRECTORY "${dst_dir}")

    add_custom_command(
            OUTPUT "${dst}"
            COMMAND ${CMAKE_COMMAND} -E copy "${src}" "${dst}"
            DEPENDS "${src}"
            COMMENT "Copying model: ${rel}"
    )

    list(APPEND COPIED_OUTPUTS "${dst}")

endforeach()
add_custom_target(CopyModels ALL
        DEPENDS ${COPIED_OUTPUTS}
)
add_dependencies(client CopyModels)

if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_CONFIGURATION_TYPES MATCHES "Release")
    add_custom_target(CopyShaders ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/Assets/Shaders"
        "${CMAKE_BINARY_DIR}/Assets/Shaders"
        COMMENT "Copying shaders for Release"
    )
    add_dependencies(client CopyShaders)

    target_compile_definitions(client PUBLIC
            SHADERS_SOURCE_DIR=L"${CMAKE_SOURCE_DIR}/Assets/Shaders"
    )
    set(CHERRYPIP_SHADERS_DIR "${CMAKE_SOURCE_DIR}/Assets/Shaders" CACHE INTERNAL "")
else()
    target_compile_definitions(client PUBLIC
        SHADERS_SOURCE_DIR=L"${CMAKE_SOURCE_DIR}/Assets/Shaders"
    )
    set(CHERRYPIP_SHADERS_DIR "${CMAKE_SOURCE_DIR}/Assets/Shaders" CACHE INTERNAL "")
endif()

target_compile_definitions(client PUBLIC
        ASSETS_SOURCE_DIR=L"${CMAKE_BINARY_DIR}/Assets"
)
set(CHERRYPIP_ASSETS_DIR "${CMAKE_BINARY_DIR}/Assets" CACHE INTERNAL "")