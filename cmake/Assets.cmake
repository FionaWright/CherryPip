if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_CONFIGURATION_TYPES MATCHES "Release")
    add_custom_target(CopyAssets ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/Assets"
        "${CMAKE_BINARY_DIR}/Assets"
        COMMENT "Copying assets for Release"
    )
    add_dependencies(CherryPip CopyAssets)

    target_compile_definitions(CherryPip PRIVATE
        ASSETS_SOURCE_DIR=L"${CMAKE_BINARY_DIR}/Assets"
    )
else()
    target_compile_definitions(CherryPip PRIVATE
        ASSETS_SOURCE_DIR=L"${CMAKE_SOURCE_DIR}/Assets"
    )
endif()