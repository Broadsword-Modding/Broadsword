# Broadsword Framework

A modern modding framework for Half Sword (UE5), built with C++20.

## Overview

Broadsword provides a complete modding API for Half Sword with:

- **Unified Thread Model**: All mod code runs on game thread - no thread safety concerns
- **Universal Keybinding**: Right-click any UI element to bind keys (automatic via source location)
- **ProcessEvent Hooks**: Intercept any UE5 function call
- **Event System**: Subscribe to framework and game events
- **Theme Support**: Alacritty TOML-based theming
- **Structured Logging**: JSON logging with frame/thread context
- **Hot-Reload Friendly**: Mods load from `Mods/` directory

## Quick Start

### Prerequisites

- Visual Studio 2022 (17.10+) with C++ Desktop Development
- CMake 3.28+
- Half Sword installed

### Build and Install

```powershell
# Clone repository
git clone https://github.com/Broadsword-Modding/Broadsword.git
cd Broadsword

# Configure and build (vcpkg auto-installs dependencies)
cmake -B build -S .
cmake --build build --config Debug

# Deploy to game
.\deploy.ps1 -Configuration Debug
```

### Launch Game

1. Start Half Sword
2. Press **INSERT** to open Broadsword menu
3. Mod windows will appear automatically

See [BUILD.md](BUILD.md) for detailed build instructions.

## Creating Mods

### Minimal Example

```cpp
#include <Broadsword.hpp>

class MyMod : public Broadsword::Mod {
public:
    void OnRegister(Broadsword::ModContext& ctx) override {
        // Register UI callback
        ctx.ui.RegisterModUI(
            "MyMod",
            "My Mod",
            [this]() { RenderUI(); }
        );

        // Subscribe to events
        ctx.events.Subscribe<OnFrameEvent>([this](auto& e) {
            OnFrame(e.frame);
        });
    }

    Broadsword::ModInfo GetInfo() const override {
        return {
            .Name = "My Mod",
            .Version = "1.0.0",
            .Author = "Me",
            .Description = "My first Broadsword mod"
        };
    }

private:
    bool m_Enabled = false;

    void RenderUI() {
        // UIContext provides wrappers that work across DLL boundaries
        if (!m_UI) return;
        m_UI->Checkbox("Enable Feature", &m_Enabled);
    }

    void OnFrame(Broadsword::Frame& frame) {
        if (m_Enabled) {
            // Do mod logic on game thread (SDK calls safe)
        }
    }

    Broadsword::UIContext* m_UI = nullptr;
};

extern "C" {
    __declspec(dllexport) Broadsword::Mod* CreateMod() {
        return new MyMod();
    }

    __declspec(dllexport) void DestroyMod(Broadsword::Mod* mod) {
        delete mod;
    }
}
```

### Key Concepts

**Thread Model**:
- All mod code runs on game thread (60 FPS)
- SDK calls are always safe
- No need for `QueueAction()` or thread synchronization

**UI Rendering**:
- Use `UIContext` wrappers (`Button`, `Checkbox`, `SliderFloat`, etc.)
- Framework wraps callbacks in ImGui windows
- Automatic keybinding via source location

**Events**:
- `OnFrameEvent` - Called every game frame
- `OnPlayerSpawnedEvent` - When player spawns
- `OnWorldLoadedEvent` - When world loads
- Custom events via EventBus

**ProcessEvent Hooks**:
```cpp
ctx.hooks.Hook("Function HalfSwordUE5.Willie_BP_C.TakeDamage",
    [](SDK::UObject* obj, SDK::UFunction* func, void* params) -> bool {
        // Return false to block original function
        // Return true to run original
        return true;
    }
);
```

## Directory Structure

```
Broadsword/
├── Foundation/          # Low-level utilities (hooks, memory, hash)
├── Engine/              # UE5 integration (SDK, ProcessEvent)
├── Services/            # Core services (UI, logging, events, config)
├── Framework/           # Framework core (ModLoader, main loop)
├── ModAPI/              # Public mod interface (Mod.hpp, ModContext.hpp)
├── Proxy/               # DLL hijacking proxy (dwmapi.dll)
├── Mods/
│   └── Enhancer/        # Example mod with 18 player mods & 8 world controls
├── docs/                # Documentation
└── deploy.ps1           # Deployment script
```

## Features

### Universal Keybinding

Right-click any UI widget to bind a key. Bindings are persistent and automatically saved.

```cpp
// Automatically bindable - right-click in-game
if (m_UI->Button("Spawn Enemy")) {
    SpawnEnemy();
}
```

### Theme System

Themes are Alacritty TOML files placed in `Themes/` directory. Select theme from settings menu.

### Structured Logging

Framework provides JSON logging with automatic context:

```cpp
frame.log.Info("Player action", {
    {"position", player->Position},
    {"health", player->Health}
});
```

Logs are written to `Broadsword_YYYY-MM-DD_HH-MM-SS.log` in game directory.

### Event System

Subscribe to events for reactive mod logic:

```cpp
ctx.events.Subscribe<OnPlayerSpawnedEvent>([](auto& e) {
    e.frame.log.Info("Player spawned");
});
```

## Architecture

**Layer 1 - Foundation**:
- DLL hijacking (dwmapi proxy)
- VMT/Inline hooks (MinHook)
- Thread-safe executor
- Hash utilities

**Layer 2 - Engine**:
- UE5 SDK (Dumper-7 generated)
- ProcessEvent hooking
- World/Player tracking

**Layer 3 - Services**:
- UIContext (ImGui wrappers + keybinding)
- Logger (structured JSON logging)
- EventBus (publish/subscribe)
- BindingManager (universal keybinding)
- Theme system

**Layer 4 - Framework**:
- ModLoader (DLL discovery & loading)
- Main loop (60 FPS game thread execution)
- Frame context (per-frame state)

**Layer 5 - ModAPI**:
- Mod interface (`IMod`)
- ModContext (dependency injection)
- Frame (per-frame API access)

## Example Mods

**Enhancer** (`Mods/Enhancer/`):
- 18 player modifications (god mode, speed, jump, damage, etc.)
- 8 world controls (time scale, gravity, etc.)
- Entity spawning (NPCs, items)
- Full UI with tabs and keybinding

## Technical Details

### SDK Integration

SDK is included in the repository (`Engine/SDK/`):
- Generated by Dumper-7 for UE5.4.4
- Each Broadsword release targets a specific game version
- Update workflow: Run Dumper-7 → Copy SDK → Build → Release

### Threading Model

```cpp
// V1 (Half-Sword-Framework) - confusing dual-thread
void OnRenderUI() {  // Render thread
    GameThread::QueueAction([]() {  // Must queue to game thread
        auto player = GetPlayer();  // SDK calls
    });
}

// V2 (Broadsword) - unified single-thread
void OnFrame(Frame& frame) {  // Game thread
    auto player = frame.world.GetPlayer();  // Always safe
    if (frame.ui.Button("Click")) { }  // Also safe
}
```

Everything runs on game thread at 60 FPS. No thread safety concerns.

### Keybinding System

Uses `std::source_location` for stable IDs:

```cpp
bool Button(std::string_view label,
            std::source_location loc = std::source_location::current());
```

ID format: `{modName}:{fileName}:{line}:{column}:{label}`

Persisted to `Broadsword.bindings.json`.

### DLL Hijacking

1. Game loads `dwmapi.dll` (Windows DWM API)
2. Proxy forwards calls to real `dwmapi.dll`
3. Proxy loads `Broadsword.dll`
4. Broadsword discovers mods in `Mods/` directory

## Dependencies

Managed via vcpkg manifest mode (`vcpkg.json`):
- **imgui** (1.91.9) - UI rendering with DX11/DX12 backends
- **minhook** (1.3.4) - Function hooking
- **nlohmann-json** (3.12.0) - JSON serialization
- **glm** (1.0.1) - Math library
- **toml11** (4.4.0) - TOML config parsing
- **fmt** (12.0.0) - String formatting

## Requirements

- **Compiler**: MSVC 2022 (17.10+) with C++20 support
- **Build System**: CMake 3.28+
- **Platform**: Windows 10/11 (x64)
- **Target**: Half Sword (UE5.4.4)

## Contributing

Contributions welcome! Areas that need work:

- Additional example mods
- Documentation improvements
- Theme collection
- Bug fixes and optimizations

## Changelog

### Current Version
- Universal keybinding system
- UIContext wrapper API
- Event-driven architecture
- ProcessEvent hooking
- Mod loader with hot-reload
- Alacritty theme support
- Structured logging

## Credits

- **Target Game**: Half Sword by Half Sword Games
- **SDK Generator**: Dumper-7
- **Predecessor**: Half-Sword-Framework (V1)

---

**Broadsword**: Modern modding framework for Half Sword
