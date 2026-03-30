if (NOT dxc_SOURCE_DIR)
    if (WIN32)
        set(DXC_FILE
            https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.8.2505/dxc_2025_05_24.zip
        )
    else ()
        set(DXC_FILE
            https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.8.2505/linux_dxc_2025_05_24.x86_64.tar.gz
        )
    endif ()

    # Download DXC using CPM
    CPMAddPackage(
        NAME dxc
        URL ${DXC_FILE}
    )
    set(dxc_SOURCE_DIR ${dxc_SOURCE_DIR} CACHE INTERNAL "")
else ()
    message(STATUS "DXC already downloaded, skipping.")
endif ()

if (WIN32)
    set(DXC_EXECUTABLE
            ${dxc_SOURCE_DIR}/bin/x64/dxc.exe
            CACHE INTERNAL "")
    set(DXC_DLLS
            ${dxc_SOURCE_DIR}/bin/x64/dxcompiler.dll
            ${dxc_SOURCE_DIR}/bin/x64/dxil.dll)
else ()
    set(DXC_EXECUTABLE
            ${dxc_SOURCE_DIR}/bin/dxc
            CACHE INTERNAL "")
    set(DXC_DLLS
            ${dxc_SOURCE_DIR}/lib/libdxcompiler.so
            ${dxc_SOURCE_DIR}/lib/libdxil.so)
endif ()