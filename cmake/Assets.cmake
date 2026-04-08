
set(TEXCONV_EXE ${CMAKE_SOURCE_DIR}/ThirdParty/texconv/texconv.exe)

# Directories containing textures
set(TEXTURE_DIRS
    "${CMAKE_SOURCE_DIR}/assets/Textures"
    "${CMAKE_SOURCE_DIR}/assets/Models"
)

set(GENERATED_DDS_FILES "")

foreach(DIR ${TEXTURE_DIRS})
    file(GLOB_RECURSE IMG_FILES
            "${DIR}/*.png"
            "${DIR}/*.jpg"
            "${DIR}/*.jpeg"
            "${DIR}/*.tga")

    foreach(IMG ${IMG_FILES})
        # Compute output path in the build directory
        file(RELATIVE_PATH REL_PATH "${CMAKE_SOURCE_DIR}" "${IMG}")
        get_filename_component(DDS_NAME "${REL_PATH}" NAME_WE)
        get_filename_component(DDS_DIR_REL "${REL_PATH}" DIRECTORY)
        set(DDS_REL "${DDS_DIR_REL}/${DDS_NAME}.dds")
        set(DDS_OUT "${CMAKE_BINARY_DIR}/${DDS_REL}")

        # Ensure output directory exists
        get_filename_component(DDS_DIR "${DDS_OUT}" DIRECTORY)
        file(MAKE_DIRECTORY "${DDS_DIR}")

        add_custom_command(
                OUTPUT "${DDS_OUT}"
                COMMAND "${TEXCONV_EXE}" -y -ft dds -f BC7_UNORM -o "${DDS_DIR}" "${IMG}"
                DEPENDS "${IMG}"
                COMMENT "Converting ${IMG} → ${DDS_OUT}"
                VERBATIM
        )

        list(APPEND GENERATED_DDS_FILES "${DDS_OUT}")
    endforeach()
endforeach()

add_custom_target(GenerateDDS ALL
        DEPENDS ${GENERATED_DDS_FILES}
)
add_dependencies(client GenerateDDS)

# All files under Assets/Models
file(GLOB_RECURSE MODEL_FILES
        "${CMAKE_SOURCE_DIR}/assets/Models/*"
        "${CMAKE_SOURCE_DIR}/assets/Resources/*"
        "${CMAKE_SOURCE_DIR}/assets/Data/*"
        "${CMAKE_SOURCE_DIR}/assets/Textures/*.hdr"
)

set(FILES_TO_COPY "")

foreach(f ${MODEL_FILES})
    # Skip PNGs
    if(f MATCHES "\\.png$")
        continue()
    endif()
    if(f MATCHES "\\.jpg$")
        continue()
    endif()
    if(f MATCHES "\\.jpeg$")
        continue()
    endif()

    list(APPEND FILES_TO_COPY "${f}")
endforeach()

set(COPIED_OUTPUTS "")

foreach(src ${FILES_TO_COPY})
    # Compute relative path under Assets/Models
    file(RELATIVE_PATH rel "${CMAKE_SOURCE_DIR}/assets/Models" "${src}")
    set(dst "${CMAKE_BINARY_DIR}/assets/Models/${rel}")

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
        "${CMAKE_SOURCE_DIR}/assets/Shaders"
        "${CMAKE_BINARY_DIR}/assets/Shaders"
        COMMENT "Copying shaders for Release"
    )
    add_dependencies(client CopyShaders)

    target_compile_definitions(client PUBLIC
            SHADERS_SOURCE_DIR=L"${CMAKE_SOURCE_DIR}/assets/Shaders"
    )
    set(CHERRYPIP_SHADERS_DIR "${CMAKE_SOURCE_DIR}/assets/Shaders" CACHE INTERNAL "")
else()
    target_compile_definitions(client PUBLIC
        SHADERS_SOURCE_DIR=L"${CMAKE_SOURCE_DIR}/assets/Shaders"
    )
    set(CHERRYPIP_SHADERS_DIR "${CMAKE_SOURCE_DIR}/assets/Shaders" CACHE INTERNAL "")
endif()

target_compile_definitions(client PUBLIC
        ASSETS_SOURCE_DIR=L"${CMAKE_BINARY_DIR}/assets"
)
set(CHERRYPIP_ASSETS_DIR "${CMAKE_BINARY_DIR}/assets" CACHE INTERNAL "")