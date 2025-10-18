# Building Broadsword

## Prerequisites

1. **Visual Studio 2022** (17.10 or later) with:
   - C++ Desktop Development workload
   - Windows 10/11 SDK
   - CMake tools

2. **vcpkg** installed and integrated
   - Install location: Can be anywhere, but you'll need the path

3. **Half Sword (Demo)** installed

## Build Steps

### 1. Configure vcpkg Toolchain

Set the vcpkg toolchain file path. Either:

**Option A: Environment Variable**
```powershell
$env:VCPKG_ROOT = "C:\path\to\vcpkg"
```

**Option B: CMake Command Line** (shown below)

### 2. Configure CMake

From the `Broadsword/` directory:

```powershell
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake"
```

Or if using the workspace vcpkg:
```powershell
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="../Half-Sword-Framework/vcpkg/scripts/buildsystems/vcpkg.cmake"
```

### 3. Build

**Debug Build:**
```powershell
cmake --build build --config Debug
```

**Release Build:**
```powershell
cmake --build build --config Release
```

### 4. Output Location

Built DLLs will be in:
- `build/bin/Debug/` or `build/bin/Release/`

You should see:
- `Broadsword.dll` - Main framework
- `dwmapi.dll` - Proxy DLL

## Installing to Half Sword

### Find Half Sword Directory

Half Sword Demo is typically installed at:
```
C:\Program Files (x86)\Steam\steamapps\common\Half Sword Demo\HalfSwordUE5\Binaries\Win64\
```

### Copy Files

Copy both DLLs to the game directory:

```powershell
# Set game path
$gamePath = "C:\Program Files (x86)\Steam\steamapps\common\Half Sword Demo\HalfSwordUE5\Binaries\Win64"

# Copy Debug build
Copy-Item build/bin/Debug/dwmapi.dll "$gamePath\"
Copy-Item build/bin/Debug/Broadsword.dll "$gamePath\"

# Or Release build
Copy-Item build/bin/Release/dwmapi.dll "$gamePath\"
Copy-Item build/bin/Release/Broadsword.dll "$gamePath\"
```

## Testing

1. Launch Half Sword
2. You should see an ImGui window titled "Broadsword Framework - Phase 1"
3. The window shows:
   - Framework status
   - Detected backend (DX11 or DX12)
   - Current FPS
   - "Phase 1 Complete!" message

4. Check the log file in the game directory:
   - `Broadsword_Phase1.log`

## Troubleshooting

### "Could not find 'Broadsword.dll'"

- Ensure both `dwmapi.dll` AND `Broadsword.dll` are in the game directory
- They must be in the same folder as `HalfSwordUE5-Win64-Shipping.exe`

### Framework doesn't initialize

- Check `Broadsword_Phase1.log` in the game directory
- Look for error messages about kiero initialization or hook failures

### No ImGui window appears

- The framework may have initialized but ImGui rendering failed
- Check the log file for backend initialization errors
- Try switching between DX11 and DX12 mode in game settings (if available)

### Build errors about missing headers

- Ensure vcpkg installed all dependencies successfully
- Try: `vcpkg install imgui[dx11-binding,dx12-binding,win32-binding] minhook nlohmann-json glm toml11 --triplet x64-windows`

### CMake can't find packages

- Verify vcpkg toolchain file path is correct
- Ensure vcpkg integration: `vcpkg integrate install`

## Development Workflow

### Quick Rebuild and Deploy

```powershell
# Build
cmake --build build --config Debug

# Deploy
$game = "C:\Program Files (x86)\Steam\steamapps\common\Half Sword Demo\HalfSwordUE5\Binaries\Win64"
Copy-Item build/bin/Debug/*.dll "$game\"

# Launch game and test
```

### Clean Build

```powershell
# Remove build directory
Remove-Item -Recurse -Force build

# Reconfigure and build
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="path/to/vcpkg/toolchain"
cmake --build build --config Debug
```

## Next Steps After Phase 1

Once Phase 1 is working (ImGui window appears), we can proceed to:
- **Phase 2**: Services layer (EventBus, Logger, Config, Input, UI)
- **Phase 3**: Engine integration (SDK, ProcessEvent hooks, World tracking)
- **Phase 4**: Mod API and mod loader
- **Phase 5**: Example mods and polish
