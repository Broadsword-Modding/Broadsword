# Broadsword Development Checkpoints

**Goal:** Reimplement Enhancer mod using full Broadsword redesign with C++26
**Strategy:** Small, incremental commits - each checkpoint is fully testable
**Status:** In Progress

---

## Progress Overview

- **Completed:** 6 / 25
- **In Progress:** Checkpoint 7
- **Current Phase:** Universal Keybinding

---

## Checkpoint List

### Phase 1: Mod API Foundation

- [x] **Checkpoint 1: Mod Base Interface**
  - Files: `ModAPI/Mod.hpp`, `ModAPI/ModInfo.hpp`
  - Commit: "feat: add Mod base interface and metadata"
  - Completed: 2025-01-19
  - Build: Verified Debug build succeeds

- [x] **Checkpoint 2: Context Objects**
  - Files: `ModAPI/Frame.hpp`, `Framework/Core/ModContext.hpp`
  - Commit: "feat: add Frame and ModContext dependency injection"
  - Completed: 2025-01-19
  - Build: Verified Debug build succeeds

- [x] **Checkpoint 3: Result Monad**
  - Files: `ModAPI/Result.hpp`, `ModAPI/Error.hpp`
  - Commit: "feat: add Result monad for type-safe error handling"
  - Completed: 2025-01-19
  - Build: Verified Debug build succeeds

### Phase 2: Event System

- [x] **Checkpoint 4: EventBus Foundation**
  - Files: `Services/EventBus/EventBus.hpp`, `Services/EventBus/EventTypes.hpp`
  - Commit: "feat: add EventBus with OnFrameEvent"
  - Completed: 2025-01-19
  - Build: Verified Debug build succeeds

- [x] **Checkpoint 5: ModLoader**
  - Files: `Framework/Core/ModLoader.hpp`, `Framework/Core/ModLoader.cpp`, `CMakeLists.txt`
  - Commit: "feat: add ModLoader with DLL discovery and lifecycle"
  - Completed: 2025-01-19
  - Build: Verified Debug build succeeds

### Phase 3: World Operations

- [x] **Checkpoint 6: WorldFacade - Part 1 (Basic)**
  - Files: `Framework/World/WorldFacade.hpp`, `Framework/World/WorldFacade.cpp`, `CMakeLists.txt`
  - Commit: "feat: add WorldFacade with basic world operations"
  - Completed: 2025-01-19
  - Build: Verified Debug build succeeds

### Phase 4: Universal Keybinding

- [ ] **Checkpoint 7: Universal Keybinding - Part 1 (BindingManager)**
  - Files: `Services/UI/BindingManager.hpp`, `Services/UI/BindingManager.cpp`
  - Commit: "feat: add BindingManager for universal keybinding"

- [ ] **Checkpoint 8: Universal Keybinding - Part 2 (UI Integration)**
  - Files: `Services/UI/UIContext.hpp`, `Services/UI/UIContext.cpp`
  - Commit: "feat: add universal keybinding to UIContext::Button()"

- [ ] **Checkpoint 9: Universal Keybinding - Part 3 (More Widgets)**
  - Files: `Services/UI/UIContext.cpp`
  - Commit: "feat: extend universal keybinding to Checkbox, Slider, Combo"

### Phase 5: C++26 Config Reflection

- [ ] **Checkpoint 10: C++26 Config Reflection - Part 1 (Attribute)**
  - Files: `Services/Config/Reflection.hpp`, `ModAPI/Config.hpp`
  - Commit: "feat: add [[configurable]] attribute for C++26 reflection"

- [ ] **Checkpoint 11: C++26 Config Reflection - Part 2 (Serialization)**
  - Files: `Services/Config/ConfigManager.hpp`, `Services/Config/ConfigManager.cpp`
  - Commit: "feat: add reflection-based config serialization"

### Phase 6: ProcessEvent Hooking

- [ ] **Checkpoint 12: ProcessEvent Hook - Part 1 (Hook Infrastructure)**
  - Files: `Engine/ProcessEventHook.hpp`, `Engine/ProcessEventHook.cpp`
  - Commit: "feat: add ProcessEvent hooking infrastructure"

- [ ] **Checkpoint 13: ProcessEvent Hook - Part 2 (Mod API)**
  - Files: `ModAPI/Hooks.hpp`
  - Commit: "feat: add ProcessEvent hook API to ModContext"

### Phase 7: Framework Integration

- [ ] **Checkpoint 14: Framework Integration**
  - Files: `Framework/Core/DllMain.cpp`
  - Commit: "feat: integrate ModLoader into framework main loop"

### Phase 8: Advanced World Operations

- [ ] **Checkpoint 15: World Facade - Part 2 (Spawning)**
  - Files: `Framework/World/ActorSpawner.hpp`, `Framework/World/ActorSpawner.cpp`
  - Commit: "feat: add ActorSpawner for type-safe spawning"

- [ ] **Checkpoint 16: World Facade - Part 3 (Queries)**
  - Files: `Framework/World/QueryEngine.hpp`, `Framework/World/QueryEngine.cpp`
  - Commit: "feat: add QueryEngine for actor searching"

### Phase 9: Public API

- [ ] **Checkpoint 17: Main API Header**
  - Files: `ModAPI/Broadsword.hpp`
  - Commit: "feat: add main Broadsword.hpp API header"

- [ ] **Checkpoint 18: Example Minimal Mod**
  - Files: `Examples/MinimalMod/MinimalMod.cpp`, `Examples/MinimalMod/CMakeLists.txt`
  - Commit: "feat: add MinimalMod example to test framework"

### Phase 10: Enhancer Mod Implementation

- [ ] **Checkpoint 19: Enhancer Mod - Project Setup**
  - Files: `Examples/EnhancerMod/EnhancerMod.hpp`, `Examples/EnhancerMod/EnhancerMod.cpp`, `Examples/EnhancerMod/CMakeLists.txt`
  - Commit: "feat: create EnhancerMod project skeleton"

- [ ] **Checkpoint 20: Enhancer - Player Tab**
  - Files: `Examples/EnhancerMod/EnhancerMod.cpp`
  - Commit: "feat: implement Enhancer player modifications tab"

- [ ] **Checkpoint 21: Enhancer - World Tab**
  - Files: `Examples/EnhancerMod/EnhancerMod.cpp`
  - Commit: "feat: implement Enhancer world controls tab"

- [ ] **Checkpoint 22: Enhancer - Spawning Tab (NPCs)**
  - Files: `Examples/EnhancerMod/EnhancerMod.cpp`
  - Commit: "feat: implement Enhancer NPC spawning"

- [ ] **Checkpoint 23: Enhancer - Spawning Tab (Items)**
  - Files: `Examples/EnhancerMod/EnhancerMod.cpp`
  - Commit: "feat: implement Enhancer item/weapon spawning"

- [ ] **Checkpoint 24: Enhancer - ProcessEvent Hooks**
  - Files: `Examples/EnhancerMod/EnhancerMod.cpp`
  - Commit: "feat: add ProcessEvent hooks for god mode"

- [ ] **Checkpoint 25: Testing & Polish**
  - Files: Bug fixes, performance testing, documentation
  - Commit: "feat: test and polish Enhancer mod"

---

## Checkpoint Details

### Checkpoint 1: Mod Base Interface
**Status:** Not Started
**Files to Create:**
- `ModAPI/Mod.hpp` - IMod base class with virtual lifecycle methods
- `ModAPI/ModInfo.hpp` - Mod metadata struct (Name, Version, Author, Description)

**Implementation:**
```cpp
// ModAPI/Mod.hpp
class Mod {
public:
    virtual ~Mod() = default;
    virtual void OnRegister(ModContext& ctx) = 0;
    virtual void OnUnregister() = 0;
    virtual ModInfo GetInfo() const = 0;
};

// ModAPI/ModInfo.hpp
struct ModInfo {
    std::string Name;
    std::string Version;
    std::string Author;
    std::string Description;
};
```

**Success Criteria:**
- Files compile without errors
- Can inherit from Mod class
- Can implement pure virtual methods

---

### Checkpoint 2: Context Objects
**Status:** Not Started
**Files to Create:**
- `ModAPI/Frame.hpp` - Frame context with dependency injection
- `Framework/Core/ModContext.hpp` - Mod registration context

**Implementation:**
```cpp
// ModAPI/Frame.hpp
struct Frame {
    WorldFacade& world;
    UIContext& ui;
    InputContext& input;
    Logger& log;
    float deltaTime;
    uint64_t frameNumber;
};

// Framework/Core/ModContext.hpp
struct ModContext {
    EventBus& events;
    UniversalConfig& config;
    Logger& log;
    HookContext& hooks;
};
```

**Success Criteria:**
- Context objects can be constructed
- References are valid
- Can pass to mod methods

---

### Checkpoint 3: Result Monad
**Status:** Not Started
**Files to Create:**
- `ModAPI/Result.hpp` - Result<T, Error> with C++26 pattern matching support
- `ModAPI/Error.hpp` - Error enum for framework errors

**Implementation:**
```cpp
// ModAPI/Error.hpp
enum class Error {
    WorldNotLoaded,
    PlayerNotFound,
    ActorNotFound,
    InvalidClass,
    SpawnFailed,
    InvalidOperation
};

// ModAPI/Result.hpp
template<typename T, typename E = Error>
class Result {
public:
    Result(T value);
    Result(E error);

    bool IsOk() const;
    bool IsError() const;
    T& Value();
    E& Error();

    explicit operator bool() const { return IsOk(); }
};
```

**Success Criteria:**
- Can construct Result with value or error
- Can check IsOk/IsError
- Can extract value or error
- Works with C++26 pattern matching

---

### Checkpoint 4: EventBus Foundation
**Status:** Not Started
**Files to Create:**
- `Services/EventBus/EventBus.hpp` - Generic pub/sub system
- `Services/EventBus/EventBus.cpp` - Implementation
- `Services/EventBus/EventTypes.hpp` - Core event type definitions

**Implementation:**
```cpp
template<typename Event>
class EventBus {
    using Callback = std::function<void(Event&)>;

    size_t Subscribe(Callback callback);
    void Unsubscribe(size_t id);
    void Emit(Event& event);
};

// EventTypes.hpp
struct OnFrameEvent {
    Frame& frame;
    float deltaTime;
};
```

**Success Criteria:**
- Can subscribe to events
- Can emit events
- Subscribers receive events
- Can unsubscribe

---

### Checkpoint 5: ModLoader
**Status:** Not Started
**Files to Create:**
- `Framework/Core/ModLoader.hpp`
- `Framework/Core/ModLoader.cpp`

**Implementation:**
```cpp
class ModLoader {
    void DiscoverMods(const std::filesystem::path& modsDir);
    void LoadMod(const std::filesystem::path& dllPath);
    void UnloadAll();

    std::vector<Mod*> GetLoadedMods() const;
};
```

**Success Criteria:**
- Can scan Mods/ folder
- Can load DLL with LoadLibrary
- Can find CreateMod/DestroyMod exports
- Can call mod lifecycle methods
- Can track loaded mods

---

### Checkpoint 6: WorldFacade - Part 1 (Basic)
**Status:** Not Started
**Files to Create:**
- `Framework/World/WorldFacade.hpp`
- `Framework/World/WorldFacade.cpp`

**Implementation:**
```cpp
class WorldFacade {
    Result<SDK::UWorld*> GetWorld();
    Result<SDK::AWillie_BP_C*> GetPlayer();
    SDK::AWorldSettings* GetWorldSettings();
};
```

**Success Criteria:**
- GetWorld() returns valid UWorld or error
- GetPlayer() returns valid player or error
- GetWorldSettings() returns settings
- Error handling works correctly

---

### Checkpoint 7: Universal Keybinding - Part 1 (BindingManager)
**Status:** Not Started
**Files to Create:**
- `Services/UI/BindingManager.hpp`
- `Services/UI/BindingManager.cpp`

**Implementation:**
```cpp
struct BindingID {
    std::string modName;
    std::string label;
    std::string context;
    size_t Hash() const;
};

class BindingManager {
    void Register(const BindingID& id, std::string_view label);
    void Bind(const BindingID& id, int vkey);
    void Unbind(const BindingID& id);
    bool WasKeyPressed(const BindingID& id);
    std::optional<int> GetKey(const BindingID& id);
    void Save(const std::filesystem::path& path);
    void Load(const std::filesystem::path& path);
};
```

**Success Criteria:**
- Can register bindings with stable IDs
- Can bind/unbind keys
- Can check key presses
- Can save/load to JSON
- No collisions with same labels

---

### Checkpoint 8: Universal Keybinding - Part 2 (UI Integration)
**Status:** Not Started
**Files to Modify:**
- `Services/UI/UIContext.hpp`
- `Services/UI/UIContext.cpp`

**Implementation:**
```cpp
class UIContext {
    bool Button(std::string_view label,
                std::source_location loc = std::source_location::current());

    // Auto-registers with BindingManager
    // Checks bound key each frame
    // Right-click opens binding popup
    // Returns true if clicked OR key pressed
};
```

**Success Criteria:**
- Button auto-registers on first call
- Right-click shows binding popup
- Bound key triggers button
- Popup allows key capture
- ESC cancels binding

---

### Checkpoint 9: Universal Keybinding - Part 3 (More Widgets)
**Status:** Not Started
**Files to Modify:**
- `Services/UI/UIContext.cpp`

**Implementation:**
```cpp
bool Checkbox(std::string_view label, bool* value);  // Toggle on key press
bool SliderFloat(std::string_view label, float* value, float min, float max);  // +/- on key
bool Combo(std::string_view label, int* current, const std::vector<std::string>& items);  // Cycle on key
```

**Success Criteria:**
- All widgets support keybinding
- Checkbox toggles on key
- Slider increments/decrements
- Combo cycles items

---

### Checkpoint 10: C++26 Config Reflection - Part 1 (Attribute)
**Status:** Not Started
**Files to Create:**
- `Services/Config/Reflection.hpp`
- `ModAPI/Config.hpp`

**Implementation:**
```cpp
// ModAPI/Config.hpp
namespace Broadsword {
    constexpr struct configurable_t {} configurable;
}

// Usage:
[[configurable]] bool m_GodMode = false;
[[configurable]] float m_Speed = 1.0f;
```

**Success Criteria:**
- Attribute compiles with C++26
- Can mark fields as configurable
- No runtime overhead

---

### Checkpoint 11: C++26 Config Reflection - Part 2 (Serialization)
**Status:** Not Started
**Files to Create:**
- `Services/Config/ConfigManager.hpp`
- `Services/Config/ConfigManager.cpp`

**Implementation:**
```cpp
template<typename T>
class ConfigManager {
    nlohmann::json Serialize(T* instance);
    void Deserialize(T* instance, const nlohmann::json& config);
};
```

**Success Criteria:**
- Can serialize [[configurable]] fields
- Can deserialize from JSON
- Type-safe (bool, int, float, string, enum)
- Handles missing fields gracefully

---

### Checkpoint 12: ProcessEvent Hook - Part 1 (Hook Infrastructure)
**Status:** Not Started
**Files to Create:**
- `Engine/ProcessEventHook.hpp`
- `Engine/ProcessEventHook.cpp`

**Implementation:**
```cpp
class ProcessEventHook {
    using HookCallback = std::function<bool(SDK::UObject*, SDK::UFunction*, void*)>;

    size_t AddHook(std::string_view functionName, HookCallback callback);
    void RemoveHook(size_t hookId);
    void Initialize();
};
```

**Success Criteria:**
- VMT hook on ProcessEvent works
- Callbacks fire for matching functions
- Return false blocks original
- Multiple hooks on same function work

---

### Checkpoint 13: ProcessEvent Hook - Part 2 (Mod API)
**Status:** Not Started
**Files to Create:**
- `ModAPI/Hooks.hpp`

**Implementation:**
```cpp
class HookContext {
    size_t Hook(std::string_view functionName, Callback callback);

    template<typename ParamsType>
    size_t Hook(std::string_view functionName,
                std::function<bool(SDK::UObject*, ParamsType*)> callback);

    void Unhook(size_t hookId);
};
```

**Success Criteria:**
- Type-safe hook registration
- Easy parameter casting
- Clean unhook on mod unload

---

### Checkpoint 14: Framework Integration
**Status:** Not Started
**Files to Modify:**
- `Framework/Core/DllMain.cpp`

**Implementation:**
- Initialize ModLoader
- Load mods from Mods/ folder
- Call OnRegister for each mod
- Fire OnFrameEvent each frame
- Build Frame context with all services
- Clean shutdown on exit

**Success Criteria:**
- Mods load on startup
- OnFrame fires every frame
- Frame context has valid references
- Mods unload cleanly

---

### Checkpoint 15: World Facade - Part 2 (Spawning)
**Status:** Not Started
**Files to Create:**
- `Framework/World/ActorSpawner.hpp`
- `Framework/World/ActorSpawner.cpp`

**Implementation:**
```cpp
class ActorSpawner {
    template<typename T>
    Result<T*> Spawn(SDK::UClass* actorClass, SDK::FVector location, SDK::FRotator rotation);
};
```

**Success Criteria:**
- Can spawn actors at location
- Type-safe return
- Error handling for spawn failures

---

### Checkpoint 16: World Facade - Part 3 (Queries)
**Status:** Not Started
**Files to Create:**
- `Framework/World/QueryEngine.hpp`
- `Framework/World/QueryEngine.cpp`

**Implementation:**
```cpp
class QueryEngine {
    template<typename T> Result<T*> FindActor();
    template<typename T> std::vector<T*> FindAllActors();
    template<typename T> std::vector<T*> FindActorsWhere(Predicate pred);
};
```

**Success Criteria:**
- Can find single actor by type
- Can find all actors of type
- Can filter with predicate

---

### Checkpoint 17: Main API Header
**Status:** Not Started
**Files to Create:**
- `ModAPI/Broadsword.hpp`

**Implementation:**
- Single include for all public APIs
- Includes Mod.hpp, Frame.hpp, Result.hpp, Config.hpp, Hooks.hpp
- Clean namespace structure

**Success Criteria:**
- Mods only need `#include <Broadsword.hpp>`
- All APIs accessible
- No conflicts

---

### Checkpoint 18: Example Minimal Mod
**Status:** Not Started
**Files to Create:**
- `Examples/MinimalMod/MinimalMod.cpp`
- `Examples/MinimalMod/CMakeLists.txt`

**Implementation:**
- Minimal mod with one button
- Tests entire pipeline
- Example for documentation

**Success Criteria:**
- Mod loads and runs
- Button works
- Keybinding works
- Config persists

---

### Checkpoint 19: Enhancer Mod - Project Setup
**Status:** Not Started
**Files to Create:**
- `Examples/EnhancerMod/EnhancerMod.hpp`
- `Examples/EnhancerMod/EnhancerMod.cpp`
- `Examples/EnhancerMod/CMakeLists.txt`

**Implementation:**
- Mod skeleton
- [[configurable]] fields
- Tab structure

**Success Criteria:**
- Project builds
- Mod loads
- Empty tabs render

---

### Checkpoint 20: Enhancer - Player Tab
**Status:** Not Started
**Files to Modify:**
- `Examples/EnhancerMod/EnhancerMod.cpp`

**Implementation:**
- Infinite stamina
- Infinite health
- Speed multiplier
- Full heal button

**Success Criteria:**
- All features work in-game
- Keybinds work
- Config persists

---

### Checkpoint 21: Enhancer - World Tab
**Status:** Not Started
**Files to Modify:**
- `Examples/EnhancerMod/EnhancerMod.cpp`

**Implementation:**
- Time scale control
- Gravity control
- World pause

**Success Criteria:**
- WorldSettings modified correctly
- Effects visible in-game

---

### Checkpoint 22: Enhancer - Spawning Tab (NPCs)
**Status:** Not Started
**Files to Modify:**
- `Examples/EnhancerMod/EnhancerMod.cpp`

**Implementation:**
- NPC type selection
- Spawn at player
- Spawn at crosshair

**Success Criteria:**
- NPCs spawn correctly
- Location accurate

---

### Checkpoint 23: Enhancer - Spawning Tab (Items)
**Status:** Not Started
**Files to Modify:**
- `Examples/EnhancerMod/EnhancerMod.cpp`

**Implementation:**
- Item spawning
- Weapon spawning
- Armor spawning

**Success Criteria:**
- Items spawn correctly
- Can pick up spawned items

---

### Checkpoint 24: Enhancer - ProcessEvent Hooks
**Status:** Not Started
**Files to Modify:**
- `Examples/EnhancerMod/EnhancerMod.cpp`

**Implementation:**
- Hook TakeDamage
- God mode blocks damage
- Damage reduction modifies params

**Success Criteria:**
- God mode works
- Damage reduction works
- No crashes

---

### Checkpoint 25: Testing & Polish
**Status:** Not Started

**Tasks:**
- Fix any remaining bugs
- Performance testing
- Documentation
- Final polish

**Success Criteria:**
- All features work
- No crashes
- Good performance
- Documented

---

## Notes

- Each checkpoint is a complete, testable unit
- Commit after each checkpoint
- Build must succeed after each checkpoint
- Test manually before moving to next checkpoint
- Update this file after completing each checkpoint

---

**Last Updated:** 2025-01-19
**Current Checkpoint:** 7 of 25
