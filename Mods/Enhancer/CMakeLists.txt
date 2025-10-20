# Enhancer Mod - Comprehensive gameplay enhancement mod

project(Enhancer)

# Source files
set(SOURCES
    EnhancerMod.cpp
    EnhancerMod.hpp
)

# Find required packages
find_package(nlohmann_json CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

# Create DLL
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/ModAPI
    ${CMAKE_SOURCE_DIR}/Engine/SDK
    ${CMAKE_SOURCE_DIR}/Services
    ${CMAKE_SOURCE_DIR}/Framework
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    nlohmann_json::nlohmann_json
    fmt::fmt
)

# Set output directory to Mods folder
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/Mods"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/Mods"
)

# C++23 for CMake compatibility, but we use /std:c++latest for C++26 features
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

# Windows-specific settings
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /std:c++latest  # C++26 features
        /W4          # Warning level 4
        /permissive- # Conformance mode
        /Zc:__cplusplus  # Correct __cplusplus macro
        /Zc:preprocessor  # Conforming preprocessor
    )
endif()
