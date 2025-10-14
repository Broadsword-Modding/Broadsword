# Broadsword Framework

A next-generation modding framework for Half Sword (UE5), designed from the ground up using modern C++26 patterns.

**Key Philosophy**: Zero Boilerplate, Maximum Power

## Features

- **Universal Keybinding**: Every UI element is automatically keybindable via right-click
- **Zero Config Code**: C++26 reflection auto-discovers and serializes config members
- **No Thread Safety Concerns**: Unified game thread model eliminates threading complexity
- **Enterprise Logging**: Structured logs with frames, threads, timing, source location
- **ProcessEvent Hooking**: Intercept any UE5 function call before execution
- **Single Config File**: All framework and mod state in one JSON file
- **Theme System**: Terminal-inspired themes with hot-reload support
- **Event-Driven**: Subscribe to events instead of manual polling
- **Modern C++26**: Pattern matching, reflection, coroutines, and more

## Quick Start

### Prerequisites

- Windows 10/11 (x64)
- Visual Studio 2022 17.10+ (MSVC 2024 for C++26 support)
- CMake 3.28+
- vcpkg

### Building

```bash
# Clone the repository
git clone https://github.com/Broadsword-Modding/Broadsword.git
cd Broadsword

# Configure with CMake
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=[path-to-vcpkg]/scripts/buildsystems/vcpkg.cmake

# Build
cmake --build build --config Release

# Output: build/bin/dwmapi.dll and build/bin/Broadsword.dll
```

### Installation

1. Copy `dwmapi.dll` and `Broadsword.dll` to `<Half Sword>\HalfSwordUE5\Binaries\Win64\`
2. Create `Mods/` folder in the same directory
3. Copy `Themes/` folder to the same directory
4. Launch Half Sword

## Creating Your First Mod

```cpp
#include <Broadsword.hpp>

class MyMod : public Broadsword::Mod {
  [[configurable]] bool m_Enabled = true;
  [[configurable]] float m_Speed = 1.0f;

  void OnRegister(Broadsword::ModContext& ctx) override {
    ctx.events.Subscribe<Broadsword::OnFrame>([this](auto& frame) {
      if (!m_Enabled) return;

      // Your mod logic here
      if (frame.input.IsKeyPressed(VK_F1)) {
        frame.log.Info("F1 pressed!");
      }
    });
  }

  void OnFrame(Broadsword::Frame& frame) override {
    // Runs every frame on game thread
    if (frame.ui.Button("My Button", VK_F2)) {  // Auto-bindable!
      frame.log.Info("Button clicked or F2 pressed!");
    }
  }
};

BROADSWORD_EXPORT_MOD(MyMod)
```

Config is automatically saved to `Broadsword.config.json`:

```json
{
  "mods": {
    "MyMod": {
      "enabled": true,
      "speed": 1.5
    }
  }
}
```

## Architecture

Broadsword uses a 5-layer architecture:

```
ModAPI         <- Mods interact here (clean, stable API)
   |
Framework      <- High-level systems (mod loader, world facade)
   |
Services       <- Core services (logging, config, events, UI)
   |
Engine         <- UE5 integration (ProcessEvent, world tracking)
   |
Foundation     <- Low-level primitives (DLL injection, hooks, memory)
```

## Universal Keybinding

Every UI element is automatically keybindable:

```cpp
// Just use UI components normally
if (frame.ui.Button("Spawn Enemy")) {
  // User can right-click this button to bind a key!
  SpawnEnemy();
}

// Sliders, checkboxes, everything is bindable
frame.ui.Checkbox("God Mode", &m_GodMode);  // Right-click to bind a toggle key
frame.ui.Slider("Speed", &m_Speed);         // Right-click to bind increment/decrement keys
```

The framework automatically:
- Generates unique IDs from source location + label
- Shows current binding or "unbound" on the button
- Presents binding popup on right-click
- Saves all bindings to config
- Handles conflicts

## ProcessEvent Hooking

Intercept any UE5 function call:

```cpp
void OnRegister(Broadsword::ModContext& ctx) override {
  m_DamageHookId = ctx.hooks.Hook(
    "Function HalfSwordUE5.Willie_BP_C.TakeDamage",
    [this](SDK::UObject* obj, SDK::UFunction* func, void* params) -> bool {
      auto* dmgParams = static_cast<SDK::FTakeDamageParams*>(params);

      if (m_GodMode) {
        return false;  // Block damage
      }

      dmgParams->DamageAmount *= 0.5f;  // Reduce damage
      return true;  // Call original
    }
  );
}
```

## Structured Logging

Enterprise-grade logging with context:

```cpp
frame.log.Info("Player spawned", {
  {"position", player->GetLocation()},
  {"health", player->Health},
  {"frame", frame.number}
});
```

Output (JSON format):

```json
{
  "timestamp": "2025-10-14T12:34:56.789Z",
  "level": "info",
  "message": "Player spawned",
  "source": "MyMod.cpp:42:OnFrame",
  "thread": "GameThread",
  "frame": 12345,
  "data": {
    "position": {"x": 100, "y": 200, "z": 300},
    "health": 100.0,
    "frame": 12345
  }
}
```

## Theme System

Themes use Alacritty TOML format:

```toml
# Themes/AyuDark.toml
[colors.primary]
background = "#0f1419"
foreground = "#e6e1cf"

[colors.normal]
black   = "#000000"
red     = "#f07178"
green   = "#c2d94c"
yellow  = "#ffb454"
blue    = "#59c2ff"
magenta = "#d2a6ff"
cyan    = "#95e6cb"
white   = "#ffffff"
```

Themes hot-reload automatically when files change.

## Documentation

- [Complete Design Specification](../BROADSWORD_REDESIGN.md) - Full technical details
- [Implementation Progress](../BROADSWORD_PROGRESS.md) - Development tracker
- [API Reference](Docs/API.md) - Complete API documentation
- [Creating Mods](Docs/CREATING_MODS.md) - Mod development guide
- [ProcessEvent Hooking](Docs/PROCESS_EVENT.md) - Function hooking guide
- [Contributing](CONTRIBUTING.md) - Development guidelines

## Examples

See the `Examples/` directory for complete example mods:

- **MinimalMod** - Basic button and checkbox
- **EnhancerMod** - Player modifications, world controls, spawning
- **EquipmentEditorMod** - Armor and weapon customization
- **DebugHelperMod** - Debug visualization tools

## Comparison to V1 (Half Sword Framework)

| Feature | V1 | Broadsword |
|---------|----|-----------|
| Language | C++20 | C++26 |
| Threading | Dual (Game + Render) | Unified (Game only) |
| Keybinding | Manual registration | Universal (auto) |
| Config | Multiple files | Single JSON |
| Mod Config | Manual JSON | Auto via reflection |
| Logging | Basic text | Structured JSON |
| Themes | None | Alacritty TOML |
| Events | Manual polling | Event bus |
| Hooks | Limited | ProcessEvent system |

## License

MIT License - See [LICENSE](LICENSE) for details

## Contributing

Contributions welcome! Please read [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## Support

- Issues: [GitHub Issues](https://github.com/Broadsword-Modding/Broadsword/issues)
- Organization: [Broadsword-Modding](https://github.com/Broadsword-Modding)

## Acknowledgments

- Half Sword by Half Sword Games
- Inspired by the Half Sword Framework V1
- Built with imgui, minhook, nlohmann-json, glm, toml11

---

**Broadsword**: A next-generation modding framework built for power and simplicity.
