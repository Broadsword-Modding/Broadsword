# Broadsword Framework - Complete Redesign Specification

## Executive Summary

**Broadsword** is a next-generation modding framework for Half Sword (UE5), designed from the ground up using modern C++26 patterns. It provides a zero-boilerplate experience for mod developers while offering enterprise-grade features like structured logging, universal keybinding, and ProcessEvent hooking.

**Key Philosophy**: "Zero Boilerplate, Maximum Power"

### What Makes Broadsword Different

1. **Universal Keybinding**: Every UI element is automatically keybindable via right-click
2. **Zero Config Code**: C++26 reflection auto-discovers and serializes config members
3. **No Thread Safety Concerns**: Unified game thread model eliminates threading complexity
4. **Enterprise Logging**: Structured logs with frames, threads, timing, source location
5. **Function Hooking**: Intercept any UE5 function call before execution
6. **Single Config File**: All framework and mod state in one JSON file
7. **Theme System**: Terminal-inspired themes with hot-reload support
8. **Event-Driven**: Subscribe to events instead of manual polling
9. **Modern C++26**: Pattern matching, reflection, coroutines, and more

---

## Repository Information

- **Organization**: `Broadsword-Modding`
- **Repository**: `https://github.com/Broadsword-Modding/Broadsword`
- **Previous Name**: Half Sword Framework (V1 - legacy, incompatible)
- **Language**: C++26
- **Platform**: Windows 10/11 (x64)
- **Compiler**: MSVC 2024 (Visual Studio 2022 17.10+)
- **Build System**: CMake 3.28+
- **Target Game**: Half Sword (UE5)

### Naming Conventions

- **Framework Name**: Broadsword Framework
- **Namespace**: `Broadsword::`
- **Main DLL**: `Broadsword.dll`
- **Proxy DLL**: `dwmapi.dll` (DLL hijacking)
- **Config File**: `Broadsword.config.json`
- **Log Files**: `Broadsword_YYYYMMDD_HHMMSS.log`
- **Theme Files**: `Themes/*.toml`
- **Mod Interface**: `Broadsword::Mod`
- **API Header**: `<Broadsword.hpp>`

### SDK Integration Strategy

**Key Decision: SDK lives IN the Broadsword repository.**

**Rationale:**
- Broadsword is tightly coupled to the SDK (exposes SDK types in public API)
- Game updates break both SDK and framework simultaneously
- Simpler for users: one download, guaranteed compatibility
- Easier development: no version coordination needed

**Version Coupling:**
- Each Broadsword release targets a specific game version
- Broadsword v2.0.0 = Half Sword Demo v1.0.0 (build 35576357)
- Broadsword v2.1.0 = Half Sword Demo v1.1.0 (build XXXXXXXX)
- Release tags clearly indicate game version: `v2.0.0-game-1.0.0-build-35576357`

**Update Workflow (when game updates):**
1. Run Dumper-7 on updated game
2. Copy generated SDK files to `Engine/SDK/Generated/`
3. Update `Engine/SDK/SDKVersion.hpp` with new game version
4. Build and test framework
5. Fix any breaking changes in framework code
6. Test all example mods
7. Release new Broadsword version (e.g., v2.1.0)

**SDK Structure:**
```
Engine/SDK/
├── Generated/              # Dumper-7 output (committed to repo)
│   ├── SDK.hpp            # Main SDK header
│   ├── Basic.hpp
│   ├── CoreUObject_*.hpp
│   ├── Engine_*.hpp
│   ├── HalfSwordUE5_*.hpp
│   └── ...
├── SDKVersion.hpp         # Auto-generated version metadata
└── Compatibility.hpp      # Future: handle minor SDK changes
```

**SDKVersion.hpp Example:**
```cpp
namespace Broadsword::SDK {
    constexpr const char* GAME_NAME = "Half Sword Demo";
    constexpr const char* GAME_VERSION = "1.0.0";
    constexpr const char* BUILD_ID = "35576357";
    constexpr const char* ENGINE_VERSION = "UE5.4.4";
    constexpr const char* SDK_GENERATED_DATE = "2025-10-14";
}
```

**Benefits:**
- No SDK download/installation step for users
- Framework and SDK versions always match
- Clear versioning: users know which game version is supported
- Simplifies CI/CD: build SDK with framework

---

## Core Philosophy

### Design Principles

1. **Simple by Default, Powerful When Needed**
   - The 90% use case should be trivial
   - The 10% edge case should be possible
   - No boilerplate for common patterns

2. **Convention Over Configuration**
   - Sensible defaults for everything
   - Minimal required setup
   - Auto-discovery where possible

3. **Explicit Dependencies**
   - No hidden global state
   - Services passed via context objects
   - Dependency injection throughout

4. **Type Safety**
   - Compile-time errors over runtime failures
   - Result<T, Error> monad for operations
   - C++26 pattern matching for error handling

5. **Developer Experience First**
   - Fast iteration (hot-reload support)
   - Excellent error messages
   - Rich debugging tools
   - Comprehensive documentation

---

## Architecture Overview

### Layer Model

Broadsword is organized into 5 distinct architectural layers:

```
┌─────────────────────────────────────────────────────────┐
│  Layer 5: Mod API (What mod developers see)             │
│  - Mod.hpp, Frame.hpp, World.hpp, UI.hpp               │
└─────────────────────────────────────────────────────────┘
                          │
┌─────────────────────────────────────────────────────────┐
│  Layer 4: Framework Core (Integration layer)            │
│  - Framework.hpp, ModLoader.hpp, FrameContext.hpp      │
└─────────────────────────────────────────────────────────┘
                          │
┌─────────────────────────────────────────────────────────┐
│  Layer 3: Services (Framework infrastructure)           │
│  - EventBus, Logger, Config, Input, UI, Theme          │
└─────────────────────────────────────────────────────────┘
                          │
┌─────────────────────────────────────────────────────────┐
│  Layer 2: Engine Integration (Game-specific)            │
│  - ProcessEventHook, WorldTracker, ActorIterator       │
└─────────────────────────────────────────────────────────┘
                          │
┌─────────────────────────────────────────────────────────┐
│  Layer 1: Foundation (Low-level primitives)             │
│  - DllHijacking, VTableHook, Memory, Threading         │
└─────────────────────────────────────────────────────────┘
```

### Directory Structure

```
Broadsword/
├── Foundation/           # Layer 1: Low-level primitives
│   ├── Injection/
│   │   ├── DllHijack.hpp
│   │   └── DllHijack.cpp
│   ├── Hooks/
│   │   ├── VTableHook.hpp
│   │   ├── VTableHook.cpp
│   │   └── InlineHook.hpp
│   ├── Memory/
│   │   ├── PatternScanner.hpp
│   │   └── MemoryUtils.hpp
│   └── Threading/
│       ├── GameThreadExecutor.hpp
│       └── GameThreadExecutor.cpp
│
├── Engine/               # Layer 2: Engine integration
│   ├── UE5/
│   │   ├── WorldTracker.hpp
│   │   ├── WorldTracker.cpp
│   │   ├── ActorIterator.hpp
│   │   └── GameStateMonitor.hpp
│   ├── SDK/
│   │   ├── Generated/        # Dumper-7 output (lives in this repo)
│   │   │   ├── SDK.hpp
│   │   │   ├── Basic.hpp
│   │   │   ├── CoreUObject_classes.hpp
│   │   │   ├── Engine_classes.hpp
│   │   │   ├── HalfSwordUE5_classes.hpp
│   │   │   └── ...
│   │   ├── SDKVersion.hpp    # Auto-generated version info
│   │   └── Compatibility.hpp # Future: multi-version support
│   ├── ProcessEventHook.hpp
│   ├── ProcessEventHook.cpp
│   └── Events/
│       ├── WorldEvents.hpp
│       ├── ActorEvents.hpp
│       └── PlayerEvents.hpp
│
├── Services/             # Layer 3: Framework services
│   ├── EventBus/
│   │   ├── EventBus.hpp
│   │   ├── EventBus.cpp
│   │   └── EventTypes.hpp
│   ├── Logging/
│   │   ├── StructuredLogger.hpp
│   │   ├── StructuredLogger.cpp
│   │   ├── LogEntry.hpp
│   │   └── LogSinks.hpp
│   ├── Config/
│   │   ├── UniversalConfig.hpp
│   │   ├── UniversalConfig.cpp
│   │   ├── Reflection.hpp
│   │   └── TypeRegistry.hpp
│   ├── Input/
│   │   ├── InputManager.hpp
│   │   ├── InputManager.cpp
│   │   └── KeyBinding.hpp
│   └── UI/
│       ├── UIContext.hpp
│       ├── UIContext.cpp
│       ├── BindingManager.hpp
│       ├── BindingManager.cpp
│       ├── Theme.hpp
│       ├── Theme.cpp
│       ├── ThemeManager.hpp
│       ├── ThemeManager.cpp
│       └── Widgets.hpp
│
├── Framework/            # Layer 4: Framework core
│   ├── Core/
│   │   ├── Framework.hpp
│   │   ├── Framework.cpp
│   │   ├── ModLoader.hpp
│   │   ├── ModLoader.cpp
│   │   ├── ModContext.hpp
│   │   └── FrameContext.hpp
│   ├── Graphics/
│   │   ├── Renderer.hpp
│   │   ├── Renderer.cpp
│   │   ├── ImGuiBackend.hpp
│   │   └── ImGuiBackend.cpp
│   └── World/
│       ├── WorldFacade.hpp
│       ├── WorldFacade.cpp
│       ├── ActorSpawner.hpp
│       └── QueryEngine.hpp
│
├── ModAPI/               # Layer 5: Public mod interface
│   ├── Broadsword.hpp            # Main API header
│   ├── Mod.hpp                   # Base Mod class
│   ├── Frame.hpp                 # Frame context
│   ├── World.hpp                 # World operations
│   ├── UI.hpp                    # UI builder
│   ├── Input.hpp                 # Input queries
│   ├── Hooks.hpp                 # ProcessEvent hooking
│   ├── Result.hpp                # Result<T, Error> monad
│   └── Utils.hpp                 # Helper functions
│
├── Proxy/                # DLL hijacking proxy
│   ├── DllMain.cpp
│   └── dwmapi.def
│
├── Themes/               # Built-in themes
│   ├── AyuDark.toml
│   ├── AyuLight.toml
│   ├── DraculaPro.toml
│   ├── TokyoNight.toml
│   └── Custom/           # User themes
│
├── Examples/             # Example mods
│   ├── MinimalMod/
│   ├── EnhancerMod/
│   ├── EquipmentEditorMod/
│   └── DebugHelperMod/
│
├── Tests/                # Unit and integration tests
│   ├── Unit/
│   └── Integration/
│
├── Docs/                 # Documentation
│   ├── API/
│   ├── Tutorials/
│   └── Migration/
│
├── CMakeLists.txt        # Root CMake file
├── vcpkg.json            # Dependencies manifest
├── README.md
├── LICENSE
└── .clang-format
```

---

## System 1: Unified Thread Model

### Problem in Legacy Systems

Many UE5 modding frameworks use a dual-thread model:
- **Game Thread**: Runs game logic, safe for SDK calls
- **Render Thread**: Runs ImGui rendering, NOT safe for SDK calls

This forces mod developers to use `GameThread::QueueAction()` from UI callbacks, which is confusing and error-prone.

### Broadsword Solution

**All mod code and SDK calls run on the game thread. ImGui rendering also happens on the game thread.**

The key insight: We separate ImGui frame building (game thread) from DirectX rendering (render thread).

```cpp
// Main framework loop (game thread)
void Framework::MainLoop() {
    while (!m_Shutdown) {
        // ALL on game thread - safe for SDK calls:

        // 1. Process queued events
        m_EventBus.ProcessQueue();

        // 2. Update game state
        m_WorldTracker.Update();

        // 3. Fire OnFrame event to mods
        Frame frame = BuildFrameContext();
        m_EventBus.Emit<OnFrameEvent>(frame);

        // 4. Build ImGui frame (still on game thread)
        //    Mods call frame.ui.Button(), etc.
        //    ImGui builds draw commands in memory
        RenderUI(frame);

        // 5. Finalize ImGui frame and queue draw data
        //    Captures ImGui::GetDrawData() for render thread
        QueueDrawData();

        // 6. Sleep for frame timing
        Sleep(16ms);  // 60 FPS target
    }
}
```

### DirectX Integration (Render Thread)

The DirectX Present hook runs on a **separate render thread** (called by GPU driver), but it only:
1. Dequeues ImGui draw data prepared by game thread
2. Submits draw data to DirectX (ImGui_ImplDX11_RenderDrawData)
3. Does **NOT** call any SDK functions
4. Does **NOT** run mod code
5. Does **NOT** call ImGui frame building functions

**Thread Boundaries:**
- **Game Thread**: Framework loop, mod lifecycle, SDK calls, ImGui frame building (ImGui::Button, etc.)
- **Render Thread**: DirectX Present hook, GPU submission only

**Result**: Mod developers never worry about thread safety - all their code runs on game thread.

### Benefits

- ✅ Zero thread safety concerns
- ✅ No `GameThread::QueueAction()` needed
- ✅ Simpler mental model
- ✅ SDK calls work everywhere
- ✅ Easier debugging

---

## System 2: Universal Keybinding

### The Vision

**Every UI element in every mod is automatically keybindable via right-click.**

Mod developers write:
```cpp
if (frame.ui.Button("Spawn Enemy")) {
    SpawnEnemy();
}
```

Users can:
1. Right-click the button
2. See "Spawn Enemy - Unbound" or "Spawn Enemy - F1"
3. Click to start binding
4. Press any key to bind
5. Key press now triggers the button

**Zero code needed from mod developer.**

### Technical Design

#### Binding Identification

Each UI element gets a unique ID that remains stable across code changes:

```cpp
struct BindingID {
    std::string modName;           // "MyMod"
    std::string label;             // "Spawn Enemy"
    std::string context;           // Optional: "PlayerTab" or "Section1"

    // Note: We do NOT use source location for the primary ID
    // because line numbers change when code is edited.
    // Instead, we use mod name + label + optional context.

    size_t Hash() const {
        return std::hash<std::string>{}(
            modName + "::" + context + "::" + label
        );
    }

    // For debugging/UI display only
    std::string sourceLocation;    // "MyMod.cpp:42:OnFrame"
};
```

**Key Design Decision**: Use `modName + context + label` for the stable ID, not source location.

**Handling Collisions**:
- If two buttons have the same label in the same context, append instance counter
- Example: "Spawn Enemy" and "Spawn Enemy##2" (ImGui-style suffix)
- Or require mod developers to use unique labels/contexts

**Migration Strategy**:
- When loading saved bindings, try to match by label first
- If label not found, show "unbound" in UI
- User rebinds with one click

#### Binding Manager

```cpp
class BindingManager {
public:
    struct Binding {
        BindingID id;
        std::optional<int> key;  // VK_* or nullopt
        std::string label;
        std::string modName;
    };

    // Register a binding (called by UI elements)
    void Register(const BindingID& id, std::string_view label);

    // Bind a key to an element
    void Bind(const BindingID& id, int vkey);

    // Unbind an element
    void Unbind(const BindingID& id);

    // Check if key was pressed this frame
    bool WasKeyPressed(const BindingID& id);

    // Get key for element
    std::optional<int> GetKey(const BindingID& id);

    // Save/load bindings
    void Save(const std::filesystem::path& path);
    void Load(const std::filesystem::path& path);

    // Conflict detection
    std::vector<BindingID> GetConflicts(int vkey);

private:
    std::unordered_map<size_t, Binding> m_Bindings;  // ID hash -> Binding
    std::unordered_map<int, std::vector<size_t>> m_KeyToIDs;  // Key -> ID hashes
    std::optional<size_t> m_WaitingForBind;  // Currently binding this ID
};
```

#### UI Context Integration

```cpp
class UIContext {
public:
    bool Button(std::string_view label,
                std::source_location loc = std::source_location::current()) {
        // Generate unique ID using STABLE elements (not source location)
        BindingID id {
            .modName = m_CurrentMod,
            .label = std::string(label),
            .context = m_CurrentContext,  // e.g., "PlayerTab"
            .sourceLocation = FormatLocation(loc)  // For debugging only
        };
        size_t hash = id.Hash();

        // Register with binding manager
        m_BindingMgr.Register(id, label);

        // Check if bound key was pressed
        bool keyPressed = m_BindingMgr.WasKeyPressed(id);

        // Render button
        bool clicked = ImGui::Button(label.data());

        // Handle right-click for binding
        if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
            OpenBindingPopup(hash, label);
        }

        // Render binding popup if active
        if (m_ActiveBindingPopup == hash) {
            RenderBindingPopup(hash, label);
        }

        return clicked || keyPressed;
    }

    // Context management for disambiguating bindings
    void PushContext(std::string_view context) {
        m_ContextStack.push_back(m_CurrentContext);
        m_CurrentContext = std::string(context);
    }

    void PopContext() {
        if (!m_ContextStack.empty()) {
            m_CurrentContext = m_ContextStack.back();
            m_ContextStack.pop_back();
        }
    }

private:
    void OpenBindingPopup(size_t hash, std::string_view label);
    void RenderBindingPopup(size_t hash, std::string_view label);

    BindingManager& m_BindingMgr;
    std::optional<size_t> m_ActiveBindingPopup;
    std::string m_CurrentMod;
};
```

#### Binding Popup UI

```cpp
void UIContext::RenderBindingPopup(size_t hash, std::string_view label) {
    std::string popupName = "binding_popup_" + std::to_string(hash);

    if (ImGui::BeginPopup(popupName.c_str())) {
        ImGui::Text("Keybind: %s", label.data());
        ImGui::Separator();

        // Show current binding
        auto binding = m_BindingMgr.GetBinding(hash);
        std::string keyText = binding && binding->key
            ? GetKeyName(*binding->key)
            : "Unbound";

        // Binding state
        bool isWaiting = m_BindingMgr.IsWaitingForBind(hash);

        if (isWaiting) {
            ImGui::TextColored({1, 1, 0, 1}, "Press any key...");
            ImGui::Text("(ESC to cancel)");

            // Capture key press
            if (auto key = m_Input.GetPressedKey()) {
                if (*key != VK_ESCAPE) {
                    // Check for conflicts
                    auto conflicts = m_BindingMgr.GetConflicts(*key);
                    if (!conflicts.empty()) {
                        ImGui::TextColored({1, 0, 0, 1},
                            "Warning: Key already bound to %zu elements",
                            conflicts.size());
                    }

                    m_BindingMgr.Bind(hash, *key);
                }
                m_BindingMgr.CancelBind();
                ImGui::CloseCurrentPopup();
            }
        } else {
            // Bind button
            if (ImGui::Button(keyText.c_str(), {150, 0})) {
                m_BindingMgr.StartBind(hash);
            }

            // Clear button
            if (binding && binding->key) {
                ImGui::SameLine();
                if (ImGui::Button("Clear")) {
                    m_BindingMgr.Unbind(hash);
                    ImGui::CloseCurrentPopup();
                }
            }
        }

        ImGui::EndPopup();
    }
}
```

### Extended Support

The system works for all UI elements:

#### Checkboxes
```cpp
bool Checkbox(std::string_view label, bool* value) {
    // Same ID system
    // Key press toggles the checkbox
}
```

#### Sliders
```cpp
bool SliderFloat(std::string_view label, float* value, float min, float max) {
    // Key press: increment by 5%
    // Shift + Key: decrement by 5%
}
```

#### Radio Buttons
```cpp
bool RadioButton(std::string_view label, int* value, int option) {
    // Key press: select this option
}
```

#### Combo Boxes
```cpp
bool Combo(std::string_view label, int* current, const std::vector<std::string>& items) {
    // Key press: cycle to next item
    // Shift + Key: cycle to previous item
}
```

### Persistence

Bindings are stored in the universal config:

```json
{
    "mods": {
        "EnhancerMod": {
            "bindings": {
                "12345678": {
                    "label": "Spawn Enemy",
                    "key": "VK_F1",
                    "location": "EnhancerMod.cpp:42:OnFrame"
                },
                "23456789": {
                    "label": "God Mode",
                    "key": "VK_F2",
                    "location": "EnhancerMod.cpp:45:OnFrame"
                }
            }
        }
    }
}
```

### Benefits

- ✅ Zero boilerplate for mod developers
- ✅ Consistent UX across all mods
- ✅ Discoverable (right-click any element)
- ✅ Automatic persistence
- ✅ Conflict detection
- ✅ No manual keybind registration

---

## System 3: C++26 Reflection Config

### The Vision

Mod developers mark config fields with an attribute, and everything is automatic:

```cpp
class MyMod : public Broadsword::Mod {
    [[configurable]] bool m_GodMode = false;
    [[configurable]] float m_Speed = 1.0f;
    [[configurable]] std::string m_PlayerName = "Default";

    // Framework automatically:
    // - Serializes to JSON
    // - Deserializes from JSON
    // - Generates UI (optional)
    // - Validates types
};
```

**Zero manual serialization code needed.**

### Technical Design (C++26 Reflection)

#### Attribute Definition

```cpp
namespace Broadsword {
    // Mark a field as configurable
    constexpr struct configurable_t {} configurable;
}
```

#### Config Manager

```cpp
template<typename T>
class ConfigManager {
public:
    // Save all [[configurable]] members
    nlohmann::json Serialize(T* instance) {
        nlohmann::json config;

        // Use C++26 reflection to iterate members
        for (auto member : std::meta::members_of(^T)) {
            // Check if member has [[configurable]] attribute
            if (HasAttribute(member, ^Broadsword::configurable)) {
                std::string name = std::meta::name_of(member);
                auto value = member.get(instance);
                config[name] = SerializeValue(value);
            }
        }

        return config;
    }

    // Load all [[configurable]] members
    void Deserialize(T* instance, const nlohmann::json& config) {
        for (auto member : std::meta::members_of(^T)) {
            if (HasAttribute(member, ^Broadsword::configurable)) {
                std::string name = std::meta::name_of(member);

                if (config.contains(name)) {
                    auto value = DeserializeValue(config[name], member.type());
                    member.set(instance, value);
                }
            }
        }
    }

private:
    // Helper: Check if member has attribute
    template<typename Member, typename Attr>
    static bool HasAttribute(Member member, Attr attr) {
        for (auto a : std::meta::attributes_of(member)) {
            if (std::meta::is_same(a, attr)) {
                return true;
            }
        }
        return false;
    }

    // Helper: Serialize value based on type
    template<typename Value>
    static nlohmann::json SerializeValue(const Value& value) {
        // Type-specific serialization
        if constexpr (std::is_same_v<Value, bool>) {
            return value;
        } else if constexpr (std::is_arithmetic_v<Value>) {
            return value;
        } else if constexpr (std::is_same_v<Value, std::string>) {
            return value;
        } else if constexpr (std::is_enum_v<Value>) {
            return static_cast<int>(value);
        }
        // ... more types
    }
};
```

#### Auto-UI Generation (Bonus)

```cpp
template<typename T>
void GenerateConfigUI(T* instance, UIContext& ui) {
    for (auto member : std::meta::members_of(^T)) {
        if (HasAttribute(member, ^Broadsword::configurable)) {
            std::string name = std::meta::name_of(member);
            auto type = member.type();

            // Generate appropriate widget
            if (std::meta::is_same(type, ^bool)) {
                bool* ptr = &member.get(instance);
                ui.Checkbox(name, ptr);
            } else if (std::meta::is_same(type, ^float)) {
                float* ptr = &member.get(instance);
                ui.SliderFloat(name, ptr, 0.0f, 10.0f);
            } else if (std::meta::is_same(type, ^int)) {
                int* ptr = &member.get(instance);
                ui.SliderInt(name, ptr, 0, 100);
            } else if (std::meta::is_same(type, ^std::string)) {
                std::string* ptr = &member.get(instance);
                ui.InputText(name, ptr);
            }
        }
    }
}
```

### Benefits

- ✅ Zero serialization boilerplate
- ✅ Type-safe
- ✅ Compile-time validation
- ✅ Auto-UI generation possible
- ✅ Easy to extend with custom attributes

---

## System 4: ProcessEvent Hooking

### Purpose

Allow mods to intercept any UE5 UFunction call before it executes.

### Architecture

```cpp
namespace Broadsword::Engine {

class ProcessEventHook {
public:
    // Hook callback signature
    // Return true to run original function, false to skip
    using HookCallback = std::function<bool(
        SDK::UObject* object,
        SDK::UFunction* function,
        void* params
    )>;

    // Add a hook for a specific function
    size_t AddHook(std::string_view functionName, HookCallback callback);

    // Remove a hook
    void RemoveHook(size_t hookId);

    // Enable/disable logging
    void EnableLogging(bool enable);

    // Set log blacklist (function names to ignore)
    void SetLogBlacklist(std::vector<std::string> blacklist);

    // Initialize (hooks all UObject vtables)
    void Initialize();

    // Shutdown (restores original vtables)
    void Shutdown();

private:
    // VMT hook detour
    static void ProcessEventDetour(
        SDK::UObject* object,
        SDK::UFunction* function,
        void* params
    );

    void HookAllVTables();
    void UnhookAllVTables();

    using ProcessEventPtr = void(__thiscall*)(SDK::UObject*, SDK::UFunction*, void*);

    std::unordered_map<std::string, std::vector<std::pair<size_t, HookCallback>>> m_Hooks;
    std::vector<std::string> m_LogBlacklist;
    ProcessEventPtr m_OriginalProcessEvent = nullptr;
    bool m_LoggingEnabled = false;
    size_t m_NextHookId = 1;
};

} // namespace Broadsword::Engine
```

**Thread Safety Model:**
- ProcessEvent is **only called on the game thread** (UE5 guarantee)
- Hook callbacks execute on the game thread (safe for SDK calls)
- Hook registration/unregistration happens during mod load/unload (also game thread)
- **No mutex needed** - everything is single-threaded on game thread
- Exception: If we add ability to register hooks at runtime from UI, those would queue and apply on next frame

```

### Mod API

```cpp
// ModAPI/Hooks.hpp
namespace Broadsword {

class HookContext {
public:
    // Register a hook
    size_t Hook(std::string_view functionName,
                std::function<bool(SDK::UObject*, SDK::UFunction*, void*)> callback);

    // Type-safe hook with parameter struct
    template<typename ParamsType>
    size_t Hook(std::string_view functionName,
                std::function<bool(SDK::UObject*, ParamsType*)> callback) {
        return Hook(functionName, [callback](auto* obj, auto* func, void* params) {
            return callback(obj, static_cast<ParamsType*>(params));
        });
    }

    // Remove a hook
    void Unhook(size_t hookId);
};

} // namespace Broadsword
```

### Usage Example

```cpp
class MyMod : public Broadsword::Mod {
    void OnRegister(ModContext& ctx) override {
        // Hook TakeDamage function
        m_DamageHookId = ctx.hooks.Hook(
            "Function HalfSwordUE5.Willie_BP_C.TakeDamage",
            [this](SDK::UObject* obj, SDK::UFunction* func, void* params) -> bool {
                auto* damageParams = static_cast<SDK::FTakeDamageParams*>(params);

                if (m_GodMode) {
                    return false;  // Don't run original (block damage)
                }

                // Reduce damage by half
                damageParams->DamageAmount *= 0.5f;

                return true;  // Run original with modified params
            }
        );

        // Hook with typed parameters
        m_DeathHookId = ctx.hooks.Hook<SDK::FDeathParams>(
            "Function HalfSwordUE5.Willie_BP_C.Die",
            [this](SDK::UObject* obj, SDK::FDeathParams* params) -> bool {
                ctx.log.Info("Player died: {}", params->DeathReason);
                return true;  // Allow death
            }
        );
    }

    void OnUnregister() override {
        ctx.hooks.Unhook(m_DamageHookId);
        ctx.hooks.Unhook(m_DeathHookId);
    }

private:
    size_t m_DamageHookId;
    size_t m_DeathHookId;
    [[configurable]] bool m_GodMode = false;
};
```

### Benefits

- ✅ Intercept any UE5 function
- ✅ Modify parameters before execution
- ✅ Prevent execution entirely
- ✅ Type-safe with parameter structs
- ✅ Multiple hooks per function
- ✅ Clean registration/unregistration

---

## System 5: Universal Config System

### Purpose

**Single source of truth for all configuration data.**

One file: `Broadsword.config.json`

### File Structure

```json
{
    "framework": {
        "version": "2.0.0",
        "logging": {
            "level": "Info",
            "console_output": true,
            "file_output": true,
            "max_file_size_mb": 50,
            "max_files": 5
        },
        "ui": {
            "theme": "Ayu Dark",
            "font_size": 14,
            "font_family": "JetBrains Mono",
            "menu_key": "VK_INSERT",
            "window_rounding": 4.0,
            "frame_rounding": 3.0
        }
    },

    "windows": {
        "MainMenu": {
            "position": [100, 100],
            "size": [800, 600],
            "visible": false,
            "pinned": false,
            "collapsed": false
        },
        "Console": {
            "position": [50, 50],
            "size": [600, 400],
            "visible": false,
            "pinned": true,
            "collapsed": false
        }
    },

    "mods": {
        "EnhancerMod": {
            "enabled": true,
            "config": {
                "m_InfiniteStamina": false,
                "m_SpeedMultiplier": 1.5,
                "m_GodMode": false
            },
            "windows": {
                "Player": {
                    "position": [200, 200],
                    "size": [400, 500],
                    "visible": false,
                    "pinned": false
                },
                "World": {
                    "position": [650, 200],
                    "size": [400, 500],
                    "visible": false,
                    "pinned": false
                }
            },
            "bindings": {
                "12345678": {
                    "label": "Spawn Enemy",
                    "key": "VK_F1",
                    "location": "EnhancerMod.cpp:42"
                },
                "23456789": {
                    "label": "God Mode",
                    "key": "VK_F2",
                    "location": "EnhancerMod.cpp:45"
                }
            }
        }
    },

    "keybinds": {
        "global": {
            "toggle_menu": "VK_INSERT",
            "toggle_console": "VK_F12"
        }
    }
}
```

### Architecture

```cpp
namespace Broadsword {

struct WindowState {
    glm::vec2 position{0, 0};
    glm::vec2 size{400, 300};
    bool visible = false;
    bool pinned = false;
    bool collapsed = false;
};

struct FrameworkSettings {
    std::string version = "2.0.0";

    struct Logging {
        std::string level = "Info";
        bool console_output = true;
        bool file_output = true;
        int max_file_size_mb = 50;
        int max_files = 5;
    } logging;

    struct UI {
        std::string theme = "Ayu Dark";
        int font_size = 14;
        std::string font_family = "JetBrains Mono";
        std::string menu_key = "VK_INSERT";
        float window_rounding = 4.0f;
        float frame_rounding = 3.0f;
    } ui;
};

struct ModConfig {
    bool enabled = true;
    nlohmann::json config;  // Reflection-generated
    std::unordered_map<std::string, WindowState> windows;
    std::unordered_map<size_t, BindingInfo> bindings;
};

class UniversalConfig {
public:
    // Load/save entire config
    void Load(const std::filesystem::path& path);
    void Save(const std::filesystem::path& path);

    // Framework settings
    FrameworkSettings& Framework();

    // Window management
    WindowState& GetWindow(std::string_view name);
    void SetWindow(std::string_view name, const WindowState& state);

    // Mod management
    ModConfig& GetMod(std::string_view name);
    void SetMod(std::string_view name, const ModConfig& config);

    // Auto-save on change
    void EnableAutoSave(bool enable);

private:
    FrameworkSettings m_Framework;
    std::unordered_map<std::string, WindowState> m_Windows;
    std::unordered_map<std::string, ModConfig> m_Mods;

    bool m_AutoSave = true;
    std::filesystem::path m_ConfigPath;
};

} // namespace Broadsword
```

### ImGui Integration

Replace ImGui's .ini file:

```cpp
void UIContext::RenderWindow(std::string_view name, std::function<void()> renderFunc) {
    auto& state = m_Config.GetWindow(name);

    // Apply saved state
    ImGui::SetNextWindowPos(ImVec2(state.position.x, state.position.y),
                           ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(state.size.x, state.size.y),
                            ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowCollapsed(state.collapsed, ImGuiCond_FirstUseEver);

    if (!state.visible && !state.pinned) {
        return;  // Don't render
    }

    ImGuiWindowFlags flags = 0;
    if (state.pinned) {
        flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    }

    if (ImGui::Begin(name.data(), &state.visible, flags)) {
        renderFunc();

        // Capture state changes
        ImVec2 pos = ImGui::GetWindowPos();
        ImVec2 size = ImGui::GetWindowSize();
        state.position = {pos.x, pos.y};
        state.size = {size.x, size.y};
        state.collapsed = ImGui::IsWindowCollapsed();
    }
    ImGui::End();
}
```

### Benefits

- ✅ Single source of truth
- ✅ No ImGui .ini file
- ✅ Easy version control
- ✅ Easy to share configs
- ✅ Cloud sync ready
- ✅ All mods use same system

---

## System 6: Enterprise Structured Logging

### Purpose

Production-grade logging with full context and performance tracking.

### Log Format

```json
{
    "timestamp": "2025-10-14T17:30:45.123456Z",
    "frame": 12345,
    "level": "INFO",
    "thread": 4512,
    "thread_name": "GameThread",
    "source": {
        "file": "MyMod.cpp",
        "line": 42,
        "function": "OnFrame"
    },
    "context": {
        "mod": "EnhancerMod",
        "category": "Player"
    },
    "message": "Player health updated",
    "data": {
        "old_health": 50.0,
        "new_health": 75.0,
        "delta": 25.0
    },
    "duration_us": 15,
    "memory_mb": 245.6
}
```

### Architecture

```cpp
namespace Broadsword {

enum class LogLevel {
    Trace,
    Debug,
    Info,
    Warn,
    Error,
    Fatal
};

struct LogEntry {
    std::chrono::system_clock::time_point timestamp;
    uint64_t frame_number;
    LogLevel level;
    uint32_t thread_id;
    std::string thread_name;

    struct SourceLocation {
        std::string file;
        int line;
        std::string function;
    } source;

    struct Context {
        std::string mod_name;
        std::string category;
        std::unordered_map<std::string, std::string> tags;
    } context;

    std::string message;
    nlohmann::json data;

    std::chrono::microseconds duration;
    size_t memory_usage_bytes;
};

class StructuredLogger {
public:
    // Main logging interface
    template<typename... Args>
    void Log(LogLevel level,
             std::string_view message,
             Args&&... args,
             std::source_location loc = std::source_location::current());

    // Convenience methods
    template<typename... Args>
    void Trace(std::string_view msg, Args&&... args);

    template<typename... Args>
    void Debug(std::string_view msg, Args&&... args);

    template<typename... Args>
    void Info(std::string_view msg, Args&&... args);

    template<typename... Args>
    void Warn(std::string_view msg, Args&&... args);

    template<typename... Args>
    void Error(std::string_view msg, Args&&... args);

    // Scoped logging for performance tracking
    class ScopedLog {
    public:
        ScopedLog(StructuredLogger& logger, std::string_view operation);
        ~ScopedLog();

        template<typename T>
        void AddData(std::string_view key, T&& value);

    private:
        StructuredLogger& m_Logger;
        std::chrono::steady_clock::time_point m_Start;
        LogEntry m_Entry;
    };

    ScopedLog ScopedOperation(std::string_view operation);

    // Context management
    void PushContext(std::string_view mod_name, std::string_view category = "");
    void PopContext();
    void AddTag(std::string_view key, std::string_view value);

    // Configuration
    void SetMinLevel(LogLevel level);
    void SetOutputs(bool console, bool file, bool in_game);
    void SetMaxFileSize(size_t bytes);
    void SetMaxFiles(int count);

    // Query logs (for in-game console)
    std::vector<LogEntry> QueryLogs(
        std::optional<LogLevel> min_level = {},
        std::optional<std::string> mod_filter = {},
        std::optional<uint64_t> frame_start = {},
        std::optional<uint64_t> frame_end = {},
        size_t max_results = 1000
    );

private:
    // Async worker
    void AsyncWriterThread();

    // Sinks
    void WriteToConsole(const LogEntry& entry);
    void WriteToFile(const LogEntry& entry);
    void WriteToInGame(const LogEntry& entry);

    // Rotation
    void RotateLogFiles();

    std::queue<LogEntry> m_Queue;
    std::mutex m_QueueMutex;
    std::condition_variable m_QueueCV;
    std::thread m_AsyncWriter;
    std::atomic<bool> m_Running{true};

    LogLevel m_MinLevel = LogLevel::Info;
    bool m_ConsoleOutput = true;
    bool m_FileOutput = true;
    bool m_InGameOutput = true;

    std::ofstream m_CurrentLogFile;
    size_t m_MaxFileSize = 50 * 1024 * 1024;  // 50 MB
    int m_MaxFiles = 5;

    std::deque<LogEntry> m_InGameBuffer;
    size_t m_MaxInGameEntries = 10000;

    static thread_local std::vector<LogEntry::Context> m_ContextStack;
};

} // namespace Broadsword
```

### Usage Examples

```cpp
// Simple logging
frame.log.Info("Player spawned");

// With format args
frame.log.Info("Player health: {}, stamina: {}", player->Health, player->Stamina);

// With structured data
frame.log.Info("Combat event",
    nlohmann::json{
        {"attacker", "Player"},
        {"target", "Enemy_01"},
        {"damage", 25.0}
    }
);

// Scoped performance tracking
{
    auto scope = frame.log.ScopedOperation("SpawnBoss");
    scope.AddData("boss_type", "Dragon");

    auto boss = SpawnBoss();

    scope.AddData("boss_id", boss->GetID());
    // Destructor logs duration
}

// Context management
frame.log.PushContext("MyMod", "Player");
frame.log.AddTag("player_id", "123");
frame.log.Info("Processing");  // Context included
frame.log.PopContext();
```

### Log File Management

```
Logs/
├── Broadsword_2025-10-14_17-30-45.log      # Current
├── Broadsword_2025-10-14_16-15-20.log
├── Broadsword_2025-10-14_15-00-10.log
├── Broadsword_2025-10-14_14-45-30.log
└── Broadsword_2025-10-14_14-30-15.log      # Oldest
```

### In-Game Console

```cpp
class ConsoleWindow {
    void Render(Frame& frame) {
        // Filters
        static LogLevel filterLevel = LogLevel::Info;
        frame.ui.ComboEnum("Min Level", &filterLevel);

        static std::string modFilter;
        frame.ui.InputText("Mod Filter", &modFilter);

        static uint64_t frameStart = 0;
        static uint64_t frameEnd = frame.frameNumber;
        frame.ui.InputUInt64("Frame Range", &frameStart, &frameEnd);

        // Query
        auto logs = frame.log.QueryLogs(filterLevel, modFilter, frameStart, frameEnd);

        // Render
        for (const auto& entry : logs) {
            ImVec4 color = GetColorForLevel(entry.level);
            frame.ui.TextColored(color,
                "[{}] [F:{}] [{}:{}] {}",
                ToString(entry.level),
                entry.frame_number,
                entry.context.mod_name,
                entry.source.line,
                entry.message
            );

            // Tooltip with structured data
            if (frame.ui.IsItemHovered() && !entry.data.empty()) {
                frame.ui.SetTooltip("{}", entry.data.dump(2));
            }
        }
    }
};
```

### Benefits

- ✅ Structured logs (JSON)
- ✅ Async I/O (no blocking)
- ✅ Frame numbers
- ✅ Thread IDs
- ✅ Source location
- ✅ Performance tracking
- ✅ In-game console
- ✅ Log rotation

---

## System 7: Theme System

### Purpose

Customizable UI appearance with terminal-inspired color schemes.

### Theme Format (Alacritty TOML)

```toml
# Themes/AyuDark.toml
name = "Ayu Dark"
author = "Ayu Theme"

[colors.primary]
foreground = "#bfbdb6"
background = "#0b0e14"

[colors.cursor]
text = "#0b0e14"
cursor = "#bfbdb6"

[colors.selection]
text = "#bfbdb6"
background = "#1b3a5b"

[colors.normal]
black = "#1e232b"
red = "#ea6c73"
green = "#7fd962"
yellow = "#f9af4f"
blue = "#53bdfa"
magenta = "#cda1fa"
cyan = "#90e1c6"
white = "#c7c7c7"

[colors.bright]
black = "#686868"
red = "#f07178"
green = "#aad94c"
yellow = "#ffb454"
blue = "#59c2ff"
magenta = "#d2a6ff"
cyan = "#95e6cb"
white = "#ffffff"
```

### Architecture

```cpp
namespace Broadsword::UI {

struct ColorScheme {
    struct Primary {
        ImVec4 foreground;
        ImVec4 background;
    } primary;

    struct Cursor {
        ImVec4 text;
        ImVec4 cursor;
    } cursor;

    struct Selection {
        ImVec4 text;
        ImVec4 background;
    } selection;

    struct Colors {
        ImVec4 black, red, green, yellow;
        ImVec4 blue, magenta, cyan, white;
    };

    Colors normal;
    Colors bright;

    static ImVec4 HexToColor(std::string_view hex);
};

struct Theme {
    std::string name;
    std::string author;
    std::filesystem::path filepath;
    ColorScheme colors;

    static std::optional<Theme> LoadFromFile(const std::filesystem::path& path);
    bool SaveToFile(const std::filesystem::path& path) const;
};

class ThemeManager {
public:
    void LoadThemes();
    std::vector<std::string> GetThemeNames() const;
    const Theme* GetTheme(std::string_view name) const;
    void ApplyTheme(std::string_view name);
    const Theme& GetCurrentTheme() const;
    void CreateTheme(const Theme& theme);
    void DeleteTheme(std::string_view name);
    void ReloadThemes();

private:
    void ApplyToImGui(const ColorScheme& colors);

    std::unordered_map<std::string, Theme> m_Themes;
    std::string m_CurrentThemeName = "Ayu Dark";
};

} // namespace Broadsword::UI
```

### Semantic Colors

```cpp
namespace Broadsword::UI {

struct Colors {
    static ImVec4 Success()  { return CurrentTheme().colors.normal.green; }
    static ImVec4 Error()    { return CurrentTheme().colors.normal.red; }
    static ImVec4 Warning()  { return CurrentTheme().colors.normal.yellow; }
    static ImVec4 Info()     { return CurrentTheme().colors.normal.blue; }
    static ImVec4 Muted()    { return CurrentTheme().colors.normal.black; }
    static ImVec4 Text()     { return CurrentTheme().colors.primary.foreground; }
    static ImVec4 Accent()   { return CurrentTheme().colors.normal.cyan; }
    static ImVec4 Highlight(){ return CurrentTheme().colors.bright.magenta; }
};

} // namespace Broadsword::UI
```

### Usage

```cpp
void OnFrame(Frame& frame) {
    // Semantic colors
    frame.ui.TextColored(UI::Colors::Success(), "Operation succeeded!");
    frame.ui.TextColored(UI::Colors::Error(), "Failed to spawn enemy");
    frame.ui.TextColored(UI::Colors::Warning(), "Low health!");

    // Direct theme access
    const auto& theme = frame.ui.GetTheme();
    frame.ui.TextColored(theme.colors.normal.magenta, "Custom");
}
```

### Built-in Themes

- Ayu Dark (default)
- Ayu Light
- Ayu Mirage
- Dracula Pro
- Tokyo Night
- One Dark
- Monokai

### Benefits

- ✅ Terminal-inspired
- ✅ Hot-reloadable
- ✅ Custom themes
- ✅ Semantic colors
- ✅ Full ImGui mapping

---

## System 8: Event-Driven Architecture

### Purpose

Replace manual polling with event subscriptions.

### Event Bus

```cpp
namespace Broadsword {

template<typename Event>
class EventBus {
public:
    using Callback = std::function<void(Event&)>;

    size_t Subscribe(Callback callback) {
        size_t id = m_NextId++;
        m_Subscribers[id] = callback;
        return id;
    }

    void Unsubscribe(size_t id) {
        m_Subscribers.erase(id);
    }

    void Emit(Event& event) {
        for (auto& [id, callback] : m_Subscribers) {
            callback(event);
        }
    }

private:
    std::unordered_map<size_t, Callback> m_Subscribers;
    size_t m_NextId = 1;
};

} // namespace Broadsword
```

### Event Types

```cpp
// Frame events
struct OnFrameEvent {
    Frame& frame;
    float deltaTime;
};

// World events
struct OnWorldLoadedEvent {
    SDK::UWorld* world;
};

struct OnWorldUnloadedEvent {
    SDK::UWorld* world;
};

// Actor events
struct OnActorSpawnedEvent {
    SDK::AActor* actor;
    SDK::UClass* actorClass;
};

// Player events
struct OnPlayerSpawnedEvent {
    SDK::AWillie_BP_C* player;
};

struct OnPlayerDiedEvent {
    SDK::AWillie_BP_C* player;
    std::string deathReason;
};
```

### Mod Usage

```cpp
class MyMod : public Broadsword::Mod {
    void OnRegister(ModContext& ctx) override {
        // Subscribe to events
        ctx.events.Subscribe<OnFrameEvent>([this](OnFrameEvent& e) {
            OnFrame(e.frame);
        });

        ctx.events.Subscribe<OnPlayerSpawnedEvent>([this](auto& e) {
            m_Player = e.player;
        });
    }

private:
    void OnFrame(Frame& frame) {
        // Per-frame logic
    }

    SDK::AWillie_BP_C* m_Player = nullptr;
};
```

### Benefits

- ✅ No manual polling
- ✅ Clear lifecycle
- ✅ Opt-in to events
- ✅ Easy to extend
- ✅ Type-safe

---

## System 9: Dependency Injection

### Purpose

Explicit dependencies instead of hidden singletons.

### Context Objects

```cpp
struct Frame {
    WorldFacade& world;
    UIContext& ui;
    InputContext& input;
    Logger& log;
    HookContext& hooks;
    float deltaTime;
    uint64_t frameNumber;
};

struct ModContext {
    EventBus& events;
    UniversalConfig& config;
    Logger& log;
    HookContext& hooks;
};
```

### Usage

```cpp
// No more Get() singletons
void OnFrame(Frame& frame) {
    frame.log.Info("Hello");        // Not Logger::Get()
    auto* player = frame.world.GetPlayer();  // Not World::Get()
    frame.ui.Button("Click");       // Not UI::Get()
}
```

### Benefits

- ✅ Explicit dependencies
- ✅ Easy testing
- ✅ No hidden state
- ✅ Clear contracts

---

## System 10: Error Handling

### Result Monad

```cpp
namespace Broadsword {

enum class Error {
    WorldNotLoaded,
    PlayerNotFound,
    ActorNotFound,
    InvalidClass,
    SpawnFailed,
    InvalidOperation,
};

template<typename T, typename E = Error>
class Result {
public:
    Result(T value) : m_Value(std::move(value)) {}
    Result(E error) : m_Value(std::move(error)) {}

    bool IsOk() const { return std::holds_alternative<T>(m_Value); }
    bool IsError() const { return std::holds_alternative<E>(m_Value); }

    T& Value() { return std::get<T>(m_Value); }
    E& Error() { return std::get<E>(m_Value); }

    explicit operator bool() const { return IsOk(); }

private:
    std::variant<T, E> m_Value;
};

} // namespace Broadsword
```

### Usage with Pattern Matching (C++26)

```cpp
Result<SDK::AWillie_BP_C*> GetPlayer() {
    if (!m_World) {
        return Error::WorldNotLoaded;
    }

    auto* player = FindActor<SDK::AWillie_BP_C>();
    if (!player) {
        return Error::PlayerNotFound;
    }

    return player;
}

void OnFrame(Frame& frame) {
    auto result = GetPlayer();

    // C++26 pattern matching
    inspect (result) {
        <Ok> player => {
            frame.log.Info("Player health: {}", player->Health);
        }
        <Error::WorldNotLoaded> => {
            frame.log.Warn("World not loaded");
        }
        <Error::PlayerNotFound> => {
            frame.log.Warn("Player not found");
        }
        _ => {
            frame.log.Error("Unknown error");
        }
    }
}
```

### Benefits

- ✅ Type-safe errors
- ✅ Explicit error handling
- ✅ Composable
- ✅ No exceptions

### Exception Boundaries

While Result<T> is preferred for expected errors, mods can still throw exceptions. The framework protects itself:

```cpp
// Framework catches all mod exceptions
void Framework::CallModLifecycle(Mod* mod, auto&& callable) {
    try {
        callable(mod);
    } catch (const std::exception& e) {
        m_Logger.Error("Mod '{}' threw exception: {}", mod->GetInfo().Name, e.what());
        m_Logger.Info("Disabling mod '{}'", mod->GetInfo().Name);
        DisableMod(mod);
    } catch (...) {
        m_Logger.Error("Mod '{}' threw unknown exception", mod->GetInfo().Name);
        DisableMod(mod);
    }
}

// Example usage
void Framework::MainLoop() {
    for (auto* mod : m_LoadedMods) {
        if (!mod->IsEnabled()) continue;

        // Protected OnFrame call
        CallModLifecycle(mod, [&](Mod* m) {
            Frame frame = BuildFrameContext();
            m->OnFrame(frame);
        });
    }
}
```

**Exception Boundaries:**
- Mod lifecycle methods (OnRegister, OnFrame, OnUnregister)
- Mod hook callbacks (ProcessEvent hooks)
- Mod event subscribers (EventBus callbacks)
- Config serialization/deserialization

**Behavior on Exception:**
- Framework logs the exception with full context
- Mod is immediately disabled (prevents further crashes)
- Other mods continue running normally
- User sees error in log console
- Next restart: mod is still disabled (user must manually re-enable)

**Config Deserialization Errors:**
```cpp
void UniversalConfig::LoadModConfig(Mod* mod) {
    try {
        nlohmann::json config = LoadFromFile("Broadsword.config.json");
        auto modConfig = config["mods"][mod->GetInfo().Name];

        // Reflection-based deserialization
        DeserializeMod(mod, modConfig);
    } catch (const nlohmann::json::exception& e) {
        m_Logger.Warn("Failed to load config for mod '{}': {}", mod->GetInfo().Name, e.what());
        m_Logger.Info("Using default config for mod '{}'", mod->GetInfo().Name);
        // Mod continues with default values
    }
}
```

---

## Mod API

### Minimal Mod Example

```cpp
#include <Broadsword.hpp>

class MyMod : public Broadsword::Mod {
public:
    // Config (auto-saved/loaded)
    [[configurable]] bool m_EnableFeature = true;
    [[configurable]] float m_Value = 1.0f;

    // Lifecycle
    void OnRegister(Broadsword::ModContext& ctx) override {
        ctx.log.Info("MyMod registered");

        ctx.events.Subscribe<Broadsword::OnFrameEvent>([this](auto& e) {
            OnFrame(e.frame);
        });
    }

    void OnUnregister() override {
        // Cleanup
    }

    // Metadata
    Broadsword::ModInfo GetInfo() const override {
        return {
            .Name = "My Mod",
            .Version = "1.0.0",
            .Author = "Me",
            .Description = "My awesome mod"
        };
    }

private:
    void OnFrame(Broadsword::Frame& frame) {
        // Every UI element is auto-bindable
        if (frame.ui.Button("Do Thing")) {
            DoThing(frame);
        }

        frame.ui.Checkbox("Enable", &m_EnableFeature);
        frame.ui.SliderFloat("Value", &m_Value, 0.0f, 10.0f);
    }

    void DoThing(Broadsword::Frame& frame) {
        auto result = frame.world.GetPlayer();
        if (result) {
            frame.log.Info("Player found!");
        } else {
            frame.log.Warn("No player: {}", result.Error());
        }
    }
};

// DLL exports
extern "C" {
    __declspec(dllexport) Broadsword::Mod* CreateMod() {
        return new MyMod();
    }

    __declspec(dllexport) void DestroyMod(Broadsword::Mod* mod) {
        delete mod;
    }
}
```

### Complex Mod Example

```cpp
class EnhancerMod : public Broadsword::Mod {
public:
    [[configurable]] bool m_InfiniteStamina = false;
    [[configurable]] bool m_InfiniteHealth = false;
    [[configurable]] float m_SpeedMultiplier = 1.0f;
    [[configurable]] float m_TimeScale = 1.0f;

    void OnRegister(Broadsword::ModContext& ctx) override {
        ctx.events.Subscribe<Broadsword::OnFrameEvent>([this](auto& e) {
            OnFrame(e.frame);
        });

        ctx.events.Subscribe<Broadsword::OnPlayerSpawnedEvent>([this](auto& e) {
            m_Player = e.player;
        });

        // Hook damage
        m_DamageHookId = ctx.hooks.Hook(
            "Function HalfSwordUE5.Willie_BP_C.TakeDamage",
            [this](SDK::UObject*, SDK::UFunction*, void* params) -> bool {
                if (m_InfiniteHealth) {
                    return false;  // Block damage
                }
                return true;
            }
        );
    }

    Broadsword::ModInfo GetInfo() const override {
        return {
            .Name = "Half Sword Enhancer",
            .Version = "2.0.0",
            .Author = "Broadsword Team",
            .Description = "Comprehensive gameplay enhancements"
        };
    }

private:
    void OnFrame(Broadsword::Frame& frame) {
        if (frame.ui.BeginTabBar("Enhancer")) {
            if (frame.ui.BeginTabItem("Player")) {
                RenderPlayerTab(frame);
                frame.ui.EndTabItem();
            }

            if (frame.ui.BeginTabItem("World")) {
                RenderWorldTab(frame);
                frame.ui.EndTabItem();
            }

            frame.ui.EndTabBar();
        }
    }

    void RenderPlayerTab(Broadsword::Frame& frame) {
        frame.ui.Checkbox("Infinite Stamina", &m_InfiniteStamina);
        frame.ui.Checkbox("Infinite Health", &m_InfiniteHealth);
        frame.ui.SliderFloat("Speed", &m_SpeedMultiplier, 0.1f, 5.0f);

        if (frame.ui.Button("Full Heal")) {
            if (m_Player) {
                m_Player->Health = 100.0;
            }
        }

        // Apply mods
        if (m_Player && m_InfiniteStamina) {
            m_Player->Stamina = 100.0;
        }
    }

    void RenderWorldTab(Broadsword::Frame& frame) {
        frame.ui.SliderFloat("Time Scale", &m_TimeScale, 0.0f, 5.0f);

        if (auto* settings = frame.world.GetWorldSettings()) {
            settings->TimeDilation = m_TimeScale;
        }
    }

    SDK::AWillie_BP_C* m_Player = nullptr;
    size_t m_DamageHookId;
};
```

---

## Build System

### CMake Structure

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.28)
project(Broadsword CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MSVC 2024 required for C++26
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
endif()

# Dependencies
find_package(imgui CONFIG REQUIRED)
find_package(minhook CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(toml11 CONFIG REQUIRED)

# Framework library
add_library(Broadsword SHARED
    Foundation/Injection/DllHijack.cpp
    Foundation/Hooks/VTableHook.cpp
    Engine/ProcessEventHook.cpp
    Services/EventBus/EventBus.cpp
    Services/Logging/StructuredLogger.cpp
    Services/Config/UniversalConfig.cpp
    Services/UI/UIContext.cpp
    Services/UI/ThemeManager.cpp
    Framework/Core/Framework.cpp
    Framework/Core/ModLoader.cpp
    # ... more files
)

target_link_libraries(Broadsword PRIVATE
    imgui::imgui
    minhook::minhook
    nlohmann_json::nlohmann_json
    glm::glm
    toml11::toml11
)

# Proxy DLL
add_library(dwmapi SHARED
    Proxy/DllMain.cpp
)

# Example mods
add_subdirectory(Examples/MinimalMod)
add_subdirectory(Examples/EnhancerMod)
```

### vcpkg Dependencies

```json
{
    "name": "broadsword",
    "version": "2.0.0",
    "dependencies": [
        {
            "name": "imgui",
            "features": ["dx11-binding", "dx12-binding", "win32-binding"]
        },
        "minhook",
        "nlohmann-json",
        "glm",
        "toml11"
    ]
}
```

### Building

```powershell
# Clone
git clone https://github.com/Broadsword-Modding/Broadsword.git
cd Broadsword

# Configure
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake

# Build
cmake --build build --config Release

# Output
# build/Release/dwmapi.dll
# build/Release/Broadsword.dll
# build/Release/Examples/*.dll
```

---

## Timeline & Milestones

### Phase 1: Foundation (4-6 weeks)
- Project setup (CMake, vcpkg)
- DLL hijacking/injection
- VMT hooking
- Unified thread model
- Memory utilities

### Phase 2: Core Services (3-4 weeks)
- Event bus
- Structured logger
- Universal config
- Input manager
- Theme system

### Phase 3: Engine Integration (3-4 weeks)
- ProcessEvent hook
- World tracker
- Actor iterator
- SDK compatibility
- Game state monitor

### Phase 4: Mod API (2-3 weeks)
- Frame/ModContext
- UIContext with keybinding
- WorldFacade
- Result<T> monad
- Coroutine support

### Phase 5: Polish (2-3 weeks)
- Example mods
- Documentation
- Performance optimization
- Testing

**Total: 10-16 weeks**

---

## Key Innovations Summary

1. **Universal Keybinding**: Right-click any UI element to bind
2. **Zero Config Code**: `[[configurable]]` attribute auto-serializes
3. **No Thread Safety**: Everything on game thread
4. **Enterprise Logging**: Structured JSON logs
5. **ProcessEvent Hooks**: Intercept any UE5 function
6. **Single Config File**: All state in `Broadsword.config.json`
7. **Theme System**: Alacritty TOML themes
8. **Event-Driven**: Subscribe instead of poll
9. **Dependency Injection**: No singletons
10. **Modern C++26**: Pattern matching, reflection, coroutines

---

## Compiler Requirements

**Target**: MSVC 2024 (Visual Studio 2022 17.10+)

C++26 features used:
- Reflection (`std::meta::`)
- Pattern matching (`inspect`)
- Improved coroutines
- `std::source_location` enhancements

---

## Repository Structure

```
github.com/Broadsword-Modding/Broadsword
├── README.md
├── LICENSE
├── BROADSWORD_REDESIGN.md (this file)
├── BROADSWORD_PROGRESS.md (tracking)
├── CMakeLists.txt
├── vcpkg.json
├── .clang-format
├── .gitignore
├── Foundation/
├── Engine/
├── Services/
├── Framework/
├── ModAPI/
├── Proxy/
├── Themes/
├── Examples/
├── Tests/
└── Docs/
```

---

**This is Broadsword: A next-generation modding framework built for power and simplicity.**
