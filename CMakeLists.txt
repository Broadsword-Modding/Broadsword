cmake_minimum_required(VERSION 3.28)
project(Broadsword VERSION 2.0.0 LANGUAGES CXX)

# C++26 Standard Required
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# MSVC-specific flags for C++26
if(MSVC)
    add_compile_options(/std:c++latest /W4 /WX- /permissive- /Zc:__cplusplus)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find vcpkg packages
find_package(imgui CONFIG REQUIRED)
find_package(minhook CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(toml11 CONFIG REQUIRED)

# Broadsword Framework Library (main DLL)
# Phase 1: Foundation Layer only
add_library(Broadsword SHARED
    # Foundation - Injection
    Foundation/Injection/DllHijack.hpp
    Foundation/Injection/DllHijack.cpp

    # Foundation - Hooks
    Foundation/Hooks/VTableHook.hpp
    Foundation/Hooks/VTableHook.cpp
    Foundation/Hooks/InlineHook.hpp

    # Foundation - Memory
    Foundation/Memory/PatternScanner.hpp
    Foundation/Memory/MemoryUtils.hpp

    # Foundation - Threading (Hook-based, single-threaded)
    Foundation/Threading/GameFrameHook.hpp
    Foundation/Threading/GameFrameHook.cpp

    # Mod API - Public Interface (Phase 1 stubs)
    ModAPI/Broadsword.hpp
    ModAPI/Mod.hpp
    ModAPI/Frame.hpp

    # Note: Engine, Services, and Framework layers will be added in Phase 2+
)

target_include_directories(Broadsword
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/ModAPI
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Foundation
        ${CMAKE_CURRENT_SOURCE_DIR}/Engine
        ${CMAKE_CURRENT_SOURCE_DIR}/Services
        ${CMAKE_CURRENT_SOURCE_DIR}/Framework
)

target_link_libraries(Broadsword
    PRIVATE
        imgui::imgui
        minhook::minhook
        nlohmann_json::nlohmann_json
        glm::glm
        toml11::toml11
        d3d11.lib
        dxgi.lib
)

target_compile_definitions(Broadsword
    PRIVATE
        BROADSWORD_EXPORTS
    PUBLIC
        BROADSWORD_VERSION_MAJOR=2
        BROADSWORD_VERSION_MINOR=0
        BROADSWORD_VERSION_PATCH=0
)

# Proxy DLL (dwmapi.dll - DLL hijacking loader)
add_library(dwmapi SHARED
    Proxy/DllMain.cpp
    Proxy/Exports.def
)

target_compile_definitions(dwmapi
    PRIVATE
        PROXY_EXPORTS
)

# Set output name to dwmapi.dll
set_target_properties(dwmapi PROPERTIES
    OUTPUT_NAME "dwmapi"
    PREFIX ""
)

# Copy Broadsword.dll next to dwmapi.dll after build
add_custom_command(TARGET Broadsword POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Broadsword>
        $<TARGET_FILE_DIR:dwmapi>
    COMMENT "Copying Broadsword.dll to proxy directory"
)

# Create Mods directory
add_custom_command(TARGET dwmapi POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:dwmapi>/Mods
    COMMENT "Creating Mods directory"
)

# Create Themes directory and copy default themes
add_custom_command(TARGET Broadsword POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:Broadsword>/Themes
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/Themes
        $<TARGET_FILE_DIR:Broadsword>/Themes
    COMMENT "Copying theme files"
)

# Installation rules
install(TARGETS Broadsword dwmapi
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ModAPI/
    DESTINATION include/Broadsword
    FILES_MATCHING PATTERN "*.hpp"
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Themes/
    DESTINATION bin/Themes
    FILES_MATCHING PATTERN "*.toml"
)

# Enable testing
enable_testing()
add_subdirectory(Tests EXCLUDE_FROM_ALL)

# Example mods (optional)
add_subdirectory(Examples EXCLUDE_FROM_ALL)

message(STATUS "Broadsword Framework v${PROJECT_VERSION}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
