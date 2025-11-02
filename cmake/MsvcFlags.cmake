# Use the Release CRT for all configurations
add_compile_options(/permissive- /Zc:preprocessor /Zc:__cplusplus)
foreach(flag_var
        CMAKE_C_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_DEBUG)
    string(REPLACE "/MDd" "/MD" ${flag_var} "${${flag_var}}")
    string(REPLACE "/MTd" "/MT" ${flag_var} "${${flag_var}}")
endforeach()