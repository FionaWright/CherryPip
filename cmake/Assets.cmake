if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_CONFIGURATION_TYPES MATCHES "Release")
    add_custom_target(CopyAssets ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/Assets"
        "${CMAKE_BINARY_DIR}/Assets"
        COMMENT "Copying assets for Release"
    )
    add_dependencies(client CopyAssets)

    target_compile_definitions(client PUBLIC
        ASSETS_SOURCE_DIR=L"${CMAKE_BINARY_DIR}/Assets"
    )
    set(CHERRYPIP_ASSETS_DIR "${CMAKE_BINARY_DIR}/Assets" CACHE INTERNAL "")
else()
    target_compile_definitions(client PUBLIC
        ASSETS_SOURCE_DIR=L"${CMAKE_SOURCE_DIR}/Assets"
    )
    set(CHERRYPIP_ASSETS_DIR "${CMAKE_SOURCE_DIR}/Assets" CACHE INTERNAL "")
endif()