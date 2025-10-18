cmake_minimum_required(VERSION 3.28)

# C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Proxy DLL - masquerades as dwmapi.dll to hijack into game
add_library(dwmapi SHARED
    DllMain.cpp
    dwmapi.def
)

target_include_directories(dwmapi
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../Foundation/Injection
)

# Output as dwmapi.dll (hijacks the real dwmapi.dll)
set_target_properties(dwmapi PROPERTIES
    OUTPUT_NAME "dwmapi"
    PREFIX ""
    SUFFIX ".dll"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Output directories for different configurations
foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${CONFIG} CONFIG_UPPER)
    set_target_properties(dwmapi PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/bin/${CONFIG}
        LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/bin/${CONFIG}
    )
endforeach()
