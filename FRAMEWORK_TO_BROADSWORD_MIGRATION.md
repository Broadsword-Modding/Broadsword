# Half Sword Framework (V1) to Broadsword (V2) Migration Map

This document maps existing Framework V1 components to their Broadsword V2 equivalents, identifying what to port, what to redesign, and what to remove.

## Component Analysis

### Core Systems

| V1 Component | V1 Location | V2 Component | V2 Location | Action | Notes |
|--------------|-------------|--------------|-------------|--------|-------|
| **DLL Hijacking** | `Proxy/` | DLL Hijacking | `Proxy/` | **Port** | Core functionality same, clean up code |
| **GameThread** | `Framework/Core/GameThread.cpp` | GameThreadExecutor | `Foundation/Threading/` | **Redesign** | Expand to full executor pattern |
| **Logger** | `Framework/Core/Logger.cpp` | StructuredLogger | `Services/Logging/` | **Replace** | V1 is basic, V2 has structured JSON logs |
| **ConfigManager** | `Framework/Core/ConfigManager.cpp` | UniversalConfig | `Services/Config/` | **Replace** | V1 uses INI, V2 uses JSON + reflection |
| **ModManager** | `Framework/Core/ModManager.cpp` | ModLoader | `Framework/Core/` | **Port + Enhance** | Add exception boundaries, event-driven |
| **KeybindManager** | `Framework/Core/KeybindManager.cpp` | BindingManager | `Services/UI/` | **Redesign** | V1 manual registration, V2 automatic |
| **MenuRegistry** | `Framework/Core/MenuRegistry.cpp` | UIContext | `Services/UI/` | **Replace** | V2 context-based, no global registry |
| **NotificationManager** | `Framework/Core/NotificationManager.cpp` | - | - | **Remove** | Use logger + in-game console instead |
| **DebugDrawing** | `Framework/Core/DebugDrawing.cpp` | DebugRenderer | `Framework/Graphics/` | **Port** | Keep this, very useful feature |

### Graphics Systems

| V1 Component | V1 Location | V2 Component | V2 Location | Action | Notes |
|--------------|-------------|--------------|-------------|--------|-------|
| **DXHook** | `Framework/Graphics/DXHook.cpp` | Renderer | `Framework/Graphics/` | **Port** | Core DirectX hooking, works well |
| **ImGuiBackend** | `Framework/Graphics/ImGuiBackend.cpp` | ImGuiBackend | `Framework/Graphics/` | **Port + Modify** | Adjust for unified thread model |
| **Renderer** | `Framework/Graphics/Renderer.cpp` | Renderer | `Framework/Graphics/` | **Port** | Main rendering loop |
| **DebugRenderer** | `Framework/Graphics/DebugRenderer.cpp` | DebugRenderer | `Framework/Graphics/` | **Port** | Frame-based debug drawing |
| **MenuBar** | `Framework/Graphics/MenuBar.cpp` | - | Built into UIContext | **Integrate** | Part of UI system, not separate |

### Game Systems

| V1 Component | V1 Location | V2 Component | V2 Location | Action | Notes |
|--------------|-------------|--------------|-------------|--------|-------|
| **GameState** | `Framework/Game/GameState.cpp` | WorldTracker | `Engine/UE5/` | **Port + Enhance** | Track world load/unload events |
| - | - | ActorIterator | `Engine/UE5/` | **New** | Safe actor iteration |
| - | - | GameStateMonitor | `Engine/UE5/` | **New** | Player spawn/death events |
| - | - | ProcessEventHook | `Engine/` | **New** | Function interception |

### Public API

| V1 Component | V1 Location | V2 Component | V2 Location | Action | Notes |
|--------------|-------------|--------------|-------------|--------|-------|
| **IMod** | `FrameworkLib/Public/IMod.h` | Mod | `ModAPI/Mod.hpp` | **Redesign** | Event-driven instead of polling |
| **ModAPI** | `FrameworkLib/Public/ModAPI.h` | Frame/ModContext | `ModAPI/` | **Redesign** | Context objects instead of singletons |
| **UI Helpers** | `FrameworkLib/Public/UI/` | UIContext | `ModAPI/UI.hpp` | **Redesign** | Context-based, auto-keybinding |
| **TMapWrapper** | `FrameworkLib/Public/Wrappers/` | - | `ModAPI/Utils.hpp` | **Port** | SDK wrappers still useful |
| **Rendering** | `FrameworkLib/Public/Rendering/` | Frame.rendering | `ModAPI/Frame.hpp` | **Integrate** | Debug drawing API |

### New V2 Systems (Not in V1)

| V2 Component | V2 Location | Purpose |
|--------------|-------------|---------|
| **EventBus** | `Services/EventBus/` | Event-driven architecture |
| **ThemeManager** | `Services/UI/` | Alacritty TOML themes |
| **InputManager** | `Services/Input/` | Centralized input handling |
| **WorldFacade** | `Framework/World/` | High-level world operations |
| **ActorSpawner** | `Framework/World/` | Safe actor spawning |
| **Result<T>** | `ModAPI/Result.hpp` | Error handling monad |

## Port Priority

### Phase 1: Foundation (Critical Path)
1. **DLL Hijacking (Proxy/)** - Port as-is, works fine
2. **GameThread** → **GameThreadExecutor** - Redesign for executor pattern
3. **DXHook** - Port DirectX hooking

### Phase 2: Core Services
4. **Logger** → **StructuredLogger** - Completely new implementation
5. **ConfigManager** → **UniversalConfig** - Completely new with reflection
6. **ModManager** → **ModLoader** - Port with enhancements

### Phase 3: Graphics
7. **ImGuiBackend** - Port with thread model adjustments
8. **Renderer** - Port main rendering
9. **DebugRenderer** - Port debug drawing

### Phase 4: API
10. **IMod** → **Mod** - Redesign for event-driven
11. **ModAPI** → **Frame/ModContext** - Redesign for contexts
12. **KeybindManager** → **BindingManager** - Redesign for auto-binding

## Breaking Changes from V1 to V2

### API Changes

**V1 (Polling-based):**
```cpp
class MyMod : public IMod {
    void OnUpdate() override {
        // Called every frame
    }

    void OnRenderUI() override {
        // Called on render thread
        // Need GameThread::QueueAction() for SDK calls
    }
};
```

**V2 (Event-driven):**
```cpp
class MyMod : public Broadsword::Mod {
    void OnRegister(ModContext& ctx) override {
        ctx.events.Subscribe<OnFrameEvent>([this](auto& e) {
            OnFrame(e.frame);
        });
    }

    void OnFrame(Frame& frame) {
        // All on game thread - SDK calls safe
        // UI rendering also here
    }
};
```

### Config Changes

**V1 (INI with manual save/load):**
```cpp
class MyMod : public IMod {
    void OnInitialize() override {
        auto& cfg = Config::Get();
        m_Speed = cfg.GetFloat("MyMod", "Speed", 1.0f);
    }

    void OnShutdown() override {
        auto& cfg = Config::Get();
        cfg.SetFloat("MyMod", "Speed", m_Speed);
        cfg.Save();
    }

    float m_Speed = 1.0f;
};
```

**V2 (Automatic with reflection):**
```cpp
class MyMod : public Broadsword::Mod {
    [[configurable]] float m_Speed = 1.0f;
    // That's it - auto-saved/loaded
};
```

### Keybinding Changes

**V1 (Manual registration):**
```cpp
void OnInitialize() override {
    auto& keybinds = KeybindManager::Get();
    m_SpawnEnemyKey = keybinds.Register("MyMod", "SpawnEnemy", VK_F1);
}

void OnUpdate() override {
    if (m_SpawnEnemyKey.IsPressed()) {
        SpawnEnemy();
    }
}
```

**V2 (Automatic):**
```cpp
void OnFrame(Frame& frame) {
    if (frame.ui.Button("Spawn Enemy")) {  // Right-click to bind
        SpawnEnemy();
    }
}
```

## Files to Port

### High Priority (Core Functionality)

```
V1 → V2 Direct Ports:

Proxy/
  DllMain.cpp              → Proxy/DllMain.cpp
  dwmapi.def               → Proxy/dwmapi.def

Framework/Graphics/
  DXHook.cpp               → Framework/Graphics/Renderer.cpp (DX hook part)
  ImGuiBackend.cpp         → Framework/Graphics/ImGuiBackend.cpp
  DebugRenderer.cpp        → Framework/Graphics/DebugRenderer.cpp

Framework/Game/
  GameState.cpp            → Engine/UE5/WorldTracker.cpp
```

### Medium Priority (Redesign Required)

```
V1 → V2 Redesigns:

Framework/Core/
  ModManager.cpp           → Framework/Core/ModLoader.cpp
  GameThread.cpp           → Foundation/Threading/GameThreadExecutor.cpp
  KeybindManager.cpp       → Services/UI/BindingManager.cpp

FrameworkLib/Public/
  IMod.h                   → ModAPI/Mod.hpp
  ModAPI.h                 → ModAPI/Frame.hpp + ModAPI/World.hpp
```

### Low Priority (Nice to Have)

```
V1 → V2 Utilities:

FrameworkLib/Public/Wrappers/
  TMapWrapper.hpp          → ModAPI/Utils.hpp (SDK helpers)
```

### Skip (Obsolete in V2)

```
Not Needed:

Framework/Core/
  NotificationManager.cpp  (use logger instead)
  MenuRegistry.cpp         (UIContext replaces this)
  ConfigManager.cpp        (UniversalConfig replaces this)
  Logger.cpp               (StructuredLogger replaces this)
```

## Dependencies to Keep

From `vcpkg.json`:
```json
{
    "dependencies": [
        "imgui[dx11-binding,win32-binding]",  // Keep
        "minhook",                             // Keep
        "nlohmann-json",                       // Keep
        "glm"                                  // Add (for math)
    ]
}
```

Add for V2:
- `toml11` - Theme parsing

## Implementation Strategy

1. **Start with working V1 code** - Copy proven implementations
2. **Adapt to V2 architecture** - Fit into new layer model
3. **Remove V1 patterns** - Replace singletons with DI, polling with events
4. **Add V2 features** - Exception boundaries, structured logging, etc.
5. **Test incrementally** - Each component works before moving to next

## Key Insights from V1

**What Worked Well:**
- DLL hijacking via dwmapi.dll - stable and reliable
- DirectX hook with kiero - works great
- ImGui integration - excellent UX
- Debug rendering - very useful for development
- Mod DLL loading - clean architecture

**What Needs Improvement:**
- Config system (INI → JSON + reflection)
- Logging (basic → structured JSON)
- Keybindings (manual → automatic)
- Thread model (dual-thread → unified)
- Error handling (crashes → exception boundaries)
- API (singletons → DI, polling → events)

## Migration Timeline

Assuming we port V1 code as base:

- **Week 1-2**: Port Foundation (DLL hijacking, threading, hooks)
- **Week 3-4**: Port Graphics (DX hook, ImGui, rendering)
- **Week 5-6**: Implement new Services (logger, config, events, UI)
- **Week 7-8**: Port ModLoader and create new ModAPI
- **Week 9-10**: Polish, testing, example mods

**Total: ~10 weeks** (matches Phase 1-4 timeline in BROADSWORD_PROGRESS.md)

---

**Conclusion:** We can leverage a lot of working V1 code, especially low-level systems (DLL hijacking, DirectX hooking, ImGui). The major work is in redesigning the services layer and public API to match Broadsword's modern architecture (events, DI, reflection, etc.).
