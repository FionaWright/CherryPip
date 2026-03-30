# Load NuGet.exe for Windows builds
function(_ww_load_nuget)
    # Latest NuGet is at https://dist.nuget.org/win-x86-commandline/latest/nuget.exe
    # Secure download with hash verification
    set(FILE_URL "https://dist.nuget.org/win-x86-commandline/latest/nuget.exe")
    set(FILE_PATH "${CMAKE_CURRENT_BINARY_DIR}/NuGet/NuGet.exe")
    file(DOWNLOAD
            ${FILE_URL}
            ${FILE_PATH}
            STATUS download_status
            LOG download_log
            TIMEOUT 300
            TLS_VERIFY ON
            TLS_VERSION 1.2
    )

    # Check download status
    list(GET download_status 0 status_code)
    if (NOT status_code EQUAL 0)
        list(GET download_status 1 status_string)
        message(FATAL_ERROR "Download failed: ${status_string}")
    else ()
        message(STATUS "File downloaded successfully to ${FILE_PATH}")
    endif ()
endfunction(_ww_load_nuget)

# Find NuGet executable
function(_ww_find_nuget)
    find_program(
            NUGET_EXE
            NAMES nuget)

    if (NOT NUGET_EXE)
        message("NUGET.EXE not found. Downloading...")
        find_program(
                NUGET_EXE
                NAMES nuget
                PATHS ${CMAKE_CURRENT_BINARY_DIR}/NuGet)

        if (NOT NUGET_EXE)
            _ww_load_nuget()
            set(NUGET_EXE "${CMAKE_CURRENT_BINARY_DIR}/NuGet/NuGet.exe" CACHE INTERNAL "Path to NuGet.exe")
        endif ()
    else ()
        message("NUGET.EXE found: ${NUGET_EXE}")
    endif ()
endfunction(_ww_find_nuget)

# Load a NuGet dependency
function(_ww_load_nuget_dependency NUGET PLUGIN_NAME ALIAS OUT_DIR)
    if (${ALIAS}_DIR)
        message("${ALIAS}_DIR already set, skipping download.")
        return()
    endif ()

    execute_process(COMMAND ${NUGET} install "${PLUGIN_NAME}" -OutputDirectory ${OUT_DIR})
    file(GLOB PLUGIN_DIRS ${OUT_DIR}/${PLUGIN_NAME}.*)
    list(LENGTH PLUGIN_DIRS PLUGIN_DIRS_L)
    if (${PLUGIN_DIRS_L} GREATER 1)
        #Sort directories by version in descending order, so the first dir is top version
        list(SORT PLUGIN_DIRS COMPARE NATURAL ORDER DESCENDING)
        list(GET PLUGIN_DIRS 0 PLUGIN_DIRX)

        #Remove older version
        MATH(EXPR PLUGIN_DIRS_L "${PLUGIN_DIRS_L}-1")
        foreach (I RANGE 1 ${PLUGIN_DIRS_L})
            list(GET PLUGIN_DIRS ${I} OLD)
            file(REMOVE_RECURSE ${OLD})
        endforeach ()
    else ()
        list(GET PLUGIN_DIRS 0 PLUGIN_DIRX)
    endif ()

    set(${ALIAS}_DIR ${PLUGIN_DIRX} CACHE STRING "${PLUGIN_NAME} PATH" FORCE)
endfunction(_ww_load_nuget_dependency)
