# Building Broadsword

## Prerequisites

1. **Visual Studio 2022** (17.10 or later) with:
   - C++ Desktop Development workload
   - Windows 10/11 SDK
   - CMake tools

2. **Git** - For cloning the repository

3. **Half Sword** - The game must be installed

## Quick Start

### 1. Clone the Repository

```powershell
git clone https://github.com/Broadsword-Modding/Broadsword.git
cd Broadsword
```

### 2. Configure and Build

Broadsword uses vcpkg in manifest mode - dependencies are automatically installed during CMake configuration.

```powershell
# Configure (vcpkg will auto-install dependencies)
cmake -B build -S .

# Build Debug
cmake --build build --config Debug

# Build Release
cmake --build build --config Release
```

### 3. Output Location

Built files will be in:
- `build/bin/Debug/` or `build/bin/Release/`

You should see:
- `Broadsword.dll` - Main framework
- `dwmapi.dll` - Proxy DLL (DLL hijacking loader)
- `Mods/` - Directory containing mod DLLs

## Installing to Half Sword

### Using the Deploy Script

The easiest way to install is using the included PowerShell script:

```powershell
# Deploy Debug build
.\deploy.ps1 -Configuration Debug

# Deploy Release build
.\deploy.ps1 -Configuration Release
```

The script will automatically:
1. Detect your Half Sword installation
2. Copy framework DLLs
3. Copy mod DLLs
4. Create Mods directory if needed

### Manual Installation

If you prefer manual installation:

1. Find your Half Sword directory (usually):
   ```
   C:\Program Files (x86)\Steam\steamapps\common\Half Sword Demo\HalfSwordUE5\Binaries\Win64\
   ```

2. Copy files:
   ```powershell
   $game = "C:\Program Files (x86)\Steam\steamapps\common\Half Sword Demo\HalfSwordUE5\Binaries\Win64"

   # Copy framework DLLs
   Copy-Item build/bin/Debug/dwmapi.dll "$game\"
   Copy-Item build/bin/Debug/Broadsword.dll "$game\"

   # Copy mods
   Copy-Item -Recurse build/bin/Debug/Mods "$game\" -Force
   ```

## Testing

1. Launch Half Sword
2. Press **INSERT** to open the Broadsword menu
3. You should see:
   - Framework information
   - Loaded mods list
   - Mod UI windows

4. Check log files in game directory:
   - `Broadsword_YYYY-MM-DD_HH-MM-SS.log`

## Building Mods

### Structure

Mods are located in `Mods/` directory. Example: `Mods/Enhancer/`

Each mod needs:
- `CMakeLists.txt` - Build configuration
- `ModName.hpp` - Mod class header
- `ModName.cpp` - Mod implementation

### Creating a New Mod

1. Create mod directory:
   ```powershell
   mkdir Mods/MyMod
   ```

2. Create `CMakeLists.txt`:
   ```cmake
   project(MyMod)

   set(SOURCES
       MyMod.cpp
       MyMod.hpp
   )

   find_package(nlohmann_json CONFIG REQUIRED)

   add_library(${PROJECT_NAME} SHARED ${SOURCES})

   target_include_directories(${PROJECT_NAME} PRIVATE
       ${CMAKE_SOURCE_DIR}/ModAPI
       ${CMAKE_SOURCE_DIR}/Engine/SDK
       ${CMAKE_SOURCE_DIR}/Services
       ${CMAKE_SOURCE_DIR}/Framework
   )

   target_link_libraries(${PROJECT_NAME} PRIVATE
       nlohmann_json::nlohmann_json
   )

   set_target_properties(${PROJECT_NAME} PROPERTIES
       RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/Mods"
   )
   ```

3. Add to root `CMakeLists.txt`:
   ```cmake
   add_subdirectory(Mods/MyMod)
   ```

4. Implement mod (see `Mods/Enhancer/` for example)

5. Build and deploy:
   ```powershell
   cmake --build build --config Debug
   .\deploy.ps1 -Configuration Debug
   ```

## Troubleshooting

### "Could not find 'Broadsword.dll'"

- Ensure both `dwmapi.dll` AND `Broadsword.dll` are in the game directory
- They must be in the same folder as `HalfSwordUE5-Win64-Shipping.exe`

### Framework doesn't initialize

- Check log file in game directory
- Look for error messages about DLL loading or hook failures
- Ensure you're using the correct architecture (x64)

### No menu appears when pressing INSERT

- Check logs for UI initialization errors
- Try switching between DX11 and DX12 in game settings
- Verify ImGui backend initialized correctly

### vcpkg dependencies fail to install

- Ensure you have internet connection (first build downloads packages)
- Check vcpkg cache: `build/vcpkg_installed/`
- Try clean build: `rm -r build; cmake -B build -S .`

### CMake configuration fails

- Verify Visual Studio 2022 is installed with C++ tools
- Ensure CMake 3.28+ is installed
- Check `vcpkg.json` is present in root directory

### Mods don't load

- Check `Mods/` directory exists in game folder
- Verify mod DLLs are x64 architecture
- Check logs for mod loading errors
- Ensure mod exports `CreateMod()` and `DestroyMod()` functions

## Development Workflow

### Quick Rebuild

```powershell
# Rebuild and deploy in one command
cmake --build build --config Debug && .\deploy.ps1 -Configuration Debug
```

### Clean Build

```powershell
# Remove build artifacts
Remove-Item -Recurse -Force build

# Reconfigure and build
cmake -B build -S .
cmake --build build --config Debug
```

### Debugging

1. Build in Debug configuration
2. Launch Half Sword
3. Visual Studio → Debug → Attach to Process
4. Select `HalfSwordUE5-Win64-Shipping.exe`
5. Set breakpoints and debug

## Dependencies

Managed automatically via vcpkg manifest (`vcpkg.json`):
- **imgui** - UI rendering (with DX11/DX12 backends)
- **minhook** - Function hooking
- **nlohmann-json** - JSON serialization
- **glm** - Math library
- **toml11** - TOML config parsing
- **fmt** - String formatting

Dependencies are installed to `build/vcpkg_installed/` on first CMake configuration.

## Architecture

```
Broadsword/
├── Foundation/          # Low-level utilities (hooks, memory)
├── Engine/              # UE5 integration (SDK, ProcessEvent)
├── Services/            # Core services (UI, logging, events)
├── Framework/           # Framework core (ModLoader, main loop)
├── ModAPI/              # Public mod interface
├── Proxy/               # DLL hijacking proxy
├── Mods/                # Example mods
│   └── Enhancer/        # Comprehensive example mod
└── build/               # Build output (generated)
```

## Advanced Configuration

### Custom vcpkg Installation

If you have a global vcpkg installation:

```powershell
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake"
```

### Build Options

```powershell
# Verbose build output
cmake --build build --config Debug --verbose

# Parallel build (faster)
cmake --build build --config Debug --parallel 8

# Build specific target
cmake --build build --config Debug --target Broadsword
cmake --build build --config Debug --target Enhancer
```

## Next Steps

- See `docs/CREATING_MODS.md` for mod development guide
- Check `Mods/Enhancer/` for a complete mod example
- Join the community for support and mod sharing
