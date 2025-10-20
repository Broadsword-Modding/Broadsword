cmake_minimum_required(VERSION 3.28)
project(Broadsword VERSION 2.0.0 LANGUAGES CXX)

# C++23 for CMake compatibility, but we use /std:c++latest for C++26 features
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# MSVC specific settings for C++26
if(MSVC)
    add_compile_options(/std:c++latest)
    add_compile_options(/W4)  # Warning level 4
    add_compile_options(/permissive-)  # Standards conformance
    add_compile_options(/Zc:__cplusplus)  # Correct __cplusplus macro

    # Enable C++26 features
    add_compile_options(/Zc:preprocessor)  # Conforming preprocessor

    # Optimization flags
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(/O2 /Ob2 /Oi /Ot /GL)
        add_link_options(/LTCG /OPT:REF /OPT:ICF)
    endif()
endif()

# vcpkg integration
find_package(imgui CONFIG REQUIRED)
find_package(minhook CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(toml11 CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

# Collect all SDK source files
file(GLOB_RECURSE SDK_SOURCES "Engine/SDK/SDK/*_functions.cpp")

# Broadsword main library
add_library(Broadsword SHARED
    # Framework Core
    Framework/Core/DllMain.cpp
    Framework/Core/ModLoader.cpp

    # Foundation - Injection
    Foundation/Injection/DllHijack.cpp

    # Foundation - Hooks
    Foundation/Hooks/VTableHook.cpp
    Foundation/Hooks/InlineHook.cpp
    Foundation/Hooks/kiero/kiero.cpp

    # Foundation - Threading
    Foundation/Threading/GameThreadExecutor.cpp

    # Graphics backends (DX11 only for Phase 1)
    Framework/Graphics/RenderBackend.cpp
    Framework/Graphics/DX11Backend.cpp
    # Framework/Graphics/DX12Backend.cpp  # TODO: Add in Phase 2

    # Services - Logging
    Services/Logging/Logger.cpp

    # Services - UI
    Services/UI/Theme.cpp
    Services/UI/UIContext.cpp
    Services/UI/BindingManager.cpp

    # Framework - UI
    Framework/UI/ConsoleWindow.cpp
    Framework/UI/SettingsWindow.cpp
    Framework/UI/AboutWindow.cpp
    Framework/UI/NotificationManager.cpp
    Framework/UI/ModMenuUI.cpp

    # Framework - World
    Framework/World/WorldFacade.cpp

    # Engine - ProcessEvent Hook
    Engine/ProcessEventHook.cpp

    # ModAPI - Hook Context
    ModAPI/HookContext.cpp

    # Engine - SDK
    Engine/SDK/SDKPch.cpp
    Engine/SDK/SDK/Basic.cpp
    ${SDK_SOURCES}
)

target_include_directories(Broadsword
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Foundation
        ${CMAKE_CURRENT_SOURCE_DIR}/Foundation/Hooks/kiero
        ${CMAKE_CURRENT_SOURCE_DIR}/Framework
        ${CMAKE_CURRENT_SOURCE_DIR}/Engine
        ${CMAKE_CURRENT_SOURCE_DIR}/Engine/SDK
        ${CMAKE_CURRENT_SOURCE_DIR}/Services
)

target_link_libraries(Broadsword
    PRIVATE
        imgui::imgui
        minhook::minhook
        nlohmann_json::nlohmann_json
        glm::glm
        toml11::toml11
        fmt::fmt
        d3d11.lib
        d3d12.lib
        dxgi.lib
)

# Set output directory
set_target_properties(Broadsword PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Proxy DLL (dwmapi.dll hijacker)
add_subdirectory(Proxy)

# Mods
add_subdirectory(Mods/Enhancer)

# Output directories for different configurations
foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${CONFIG} CONFIG_UPPER)
    set_target_properties(Broadsword PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/bin/${CONFIG}
        LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/bin/${CONFIG}
    )
endforeach()
