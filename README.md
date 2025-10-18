# Broadsword Framework

A next-generation modding framework for Half Sword (UE5), built with modern C++26.

## Overview

Broadsword is a complete redesign of the Half Sword Framework (V1), featuring:

- **Zero Boilerplate**: Automatic keybinding, config serialization via C++26 reflection
- **Unified Thread Model**: All mod code runs on game thread - no thread safety concerns
- **Enterprise Logging**: Structured JSON logs with frames, threads, timing
- **ProcessEvent Hooks**: Intercept any UE5 function call
- **Theme System**: Terminal-inspired Alacritty TOML themes
- **Event-Driven**: Subscribe to events instead of manual polling
- **Modern C++26**: Pattern matching, reflection, coroutines

## Project Status

**Current Phase**: Design Review Complete

This project is currently in the design phase. All core systems are specified and ready for implementation.

## Documentation

- **[BROADSWORD_REDESIGN.md](BROADSWORD_REDESIGN.md)** - Complete technical specification
- **[BROADSWORD_PROGRESS.md](BROADSWORD_PROGRESS.md)** - Implementation progress tracker
- **[FRAMEWORK_TO_BROADSWORD_MIGRATION.md](FRAMEWORK_TO_BROADSWORD_MIGRATION.md)** - V1 to V2 migration guide

## Directory Structure

```
Broadsword/
├── Foundation/          # Layer 1: DLL hijacking, hooks, threading
├── Engine/              # Layer 2: UE5 integration, SDK, ProcessEvent
├── Services/            # Layer 3: EventBus, Logger, Config, UI, Input
├── Framework/           # Layer 4: Framework core, ModLoader
├── ModAPI/              # Layer 5: Public mod interface
├── Proxy/               # DLL hijacking proxy (dwmapi.dll)
├── Themes/              # Alacritty TOML themes
├── Examples/            # Example mods
├── Tests/               # Unit and integration tests
└── Docs/                # Additional documentation
```

## Key Features

### 1. Universal Keybinding
Right-click any UI element to bind a key - completely automatic:

```cpp
if (frame.ui.Button("Spawn Enemy")) {  // Right-click → bind key
    SpawnEnemy();
}
```

### 2. Zero-Config Code
Mark fields with `[[configurable]]` for automatic serialization:

```cpp
class MyMod : public Broadsword::Mod {
    [[configurable]] bool m_GodMode = false;
    [[configurable]] float m_Speed = 1.0f;
    // Auto-saved to Broadsword.config.json
};
```

### 3. Unified Thread Model
No more thread safety concerns:

```cpp
void OnFrame(Frame& frame) {
    // All on game thread - SDK calls safe everywhere
    auto* player = frame.world.GetPlayer();
    frame.ui.Button("Click");
}
```

### 4. ProcessEvent Hooking
Intercept any UE5 function:

```cpp
ctx.hooks.Hook("Function HalfSwordUE5.Willie_BP_C.TakeDamage",
    [](SDK::UObject* obj, SDK::UFunction* func, void* params) -> bool {
        if (m_GodMode) return false;  // Block damage
        return true;  // Run original
    }
);
```

### 5. Structured Logging
Enterprise-grade logs:

```cpp
frame.log.Info("Player spawned", nlohmann::json{
    {"position", player->Position},
    {"health", player->Health}
});
```

## Minimal Mod Example

```cpp
#include <Broadsword.hpp>

class MyMod : public Broadsword::Mod {
    [[configurable]] bool m_Feature = true;

    void OnRegister(ModContext& ctx) override {
        ctx.events.Subscribe<OnFrameEvent>([this](auto& e) {
            OnFrame(e.frame);
        });
    }

    Broadsword::ModInfo GetInfo() const override {
        return {
            .Name = "My Mod",
            .Version = "1.0.0",
            .Author = "Me"
        };
    }

private:
    void OnFrame(Frame& frame) {
        if (frame.ui.Button("Do Thing")) {
            frame.log.Info("Button clicked!");
        }
    }
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

## Design Improvements Over V1

### Fixed Issues from V1
1. **Threading Model**: V1 had confusing dual-thread model requiring `GameThread::QueueAction()`. V2 runs everything on game thread.
2. **Keybinding IDs**: V1 used source location which broke on code changes. V2 uses stable label-based IDs.
3. **ProcessEvent Thread Safety**: V1 used mutex unnecessarily. V2 clarifies single-threaded execution.
4. **Exception Handling**: V1 could crash entire game. V2 has exception boundaries that disable crashing mods.
5. **SDK Coupling**: V1 had separate SDK package. V2 includes SDK in repo for guaranteed compatibility.

### New Features in V2
- C++26 reflection for automatic config serialization
- Event-driven architecture (EventBus)
- Universal keybinding system
- Theme system with Alacritty TOML
- Structured logging with JSON
- Result<T> monad for error handling
- Single config file for all state

## Requirements

- **Compiler**: MSVC 2024 (Visual Studio 2022 17.10+) with C++26 support
- **Build System**: CMake 3.28+
- **Platform**: Windows 10/11 (x64)
- **Target**: Half Sword (UE5.4.4)

## C++26 Features Used

- **Reflection** (`std::meta::`) - For automatic config serialization
- **Pattern Matching** (`inspect`) - For Result<T> error handling
- **Improved Coroutines** - For async operations
- **std::source_location** - For debugging context

## SDK Integration

The SDK is included directly in this repository:
- Each Broadsword release targets a specific game version
- No separate SDK download needed
- Guaranteed framework/SDK compatibility

**Update workflow when game updates:**
1. Run Dumper-7 on updated game
2. Copy SDK to `Engine/SDK/Generated/`
3. Update `SDKVersion.hpp`
4. Build, test, release new Broadsword version

## Next Steps

Ready to begin implementation! See [BROADSWORD_PROGRESS.md](BROADSWORD_PROGRESS.md) for phase breakdown.

**Phase 1 (4-6 weeks)**: Foundation layer
- DLL hijacking
- VMT hooks
- Threading system
- Memory utilities

## Contributing

This is a greenfield project - all major systems need implementation. See the progress tracker for current status and open tasks.

## License

TBD (will match Half Sword Framework V1 license)

## Credits

- **Predecessor**: Half Sword Framework V1
- **Target Game**: Half Sword by Half Sword Games
- **SDK Generator**: Dumper-7

---

**Broadsword**: Next-generation modding framework for Half Sword
