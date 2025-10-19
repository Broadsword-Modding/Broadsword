# Half Sword SDK - Broadsword Integration

This directory contains the Dumper-7 generated Unreal Engine 5 SDK for Half Sword, integrated directly into the Broadsword framework.

## SDK Information

- **Game:** Half Sword
- **Engine Version:** Unreal Engine 5.4.4
- **Generator:** Dumper-7
- **GWorld Offset:** `0x07DC0AF0` (view in About window)

## Integration Status

The SDK is **fully integrated** into Broadsword's CMake build system. You do not need to manually configure anything.

### What's Already Set Up

✅ All SDK source files (`*_functions.cpp`) are automatically compiled
✅ `SDKPch.cpp` (precompiled header) is included
✅ `Basic.cpp` (core SDK utilities) is included
✅ Include directories are properly configured
✅ No manual project setup required

## Using the SDK in Broadsword Mods

### Include Headers

Include only what you need for faster compilation:

```cpp
#include "Engine/SDK/SDK/Basic.hpp"           // Core types and utilities
#include "Engine/SDK/SDK/Engine_classes.hpp"  // Engine classes (UWorld, APlayerController, etc.)
#include "Engine/SDK/SDK/CoreUObject_classes.hpp" // Core UObject classes
```

Or include everything (slower):

```cpp
#include "Engine/SDK/SDK.hpp"  // All SDK headers
```

### Critical Files (Already Included)

These files are automatically included in the Broadsword build - **you don't need to add them**:

1. **SDKPch.cpp** - Precompiled header source
2. **Basic.cpp** - Contains essential SDK utilities:
   - `SDK::InSDKUtils::GetImageBase()` - Gets game base address
   - `SDK::BasicFilesImpleUtils::FindClassByName()` - Finds UClass by name
   - `SDK::BasicFilesImpleUtils::GetObjectIndex()` - Gets object array index
   - `SDK::BasicFilesImpleUtils::GetObjFNameAsUInt64()` - Gets object FName as uint64
   - `SDK::BasicFilesImpleUtils::GetObjectByIndex()` - Gets object from array

3. **All *_functions.cpp files** - Automatically discovered via CMake glob pattern

## Broadsword Mod API - No Direct SDK Access Needed

**Important**: In Broadsword's redesigned architecture, you typically don't call SDK functions directly. Instead, use the high-level APIs provided through the `Frame` and `ModContext` objects.

### Mod Lifecycle

```cpp
#include <Broadsword.hpp>

class MyMod : public Broadsword::Mod {
public:
    // Called once when mod is loaded
    void OnRegister(Broadsword::ModContext& ctx) override {
        ctx.log.Info("MyMod registered");

        // Subscribe to frame events
        ctx.events.Subscribe<Broadsword::OnFrameEvent>([this](auto& e) {
            OnFrame(e.frame);
        });
    }

    // Called once when mod is unloaded
    void OnUnregister() override {
        // Cleanup
    }

    // Mod metadata
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
        // Use frame.world, frame.ui, frame.input, frame.log
        // All SDK access happens through these facades
    }
};
```

### Accessing the SDK Through Frame Context

The `Frame` object provides safe, high-level access to game state:

```cpp
void OnFrame(Broadsword::Frame& frame) {
    // Get player through WorldFacade (not direct SDK)
    auto result = frame.world.GetPlayer();
    if (result) {
        SDK::AWillie_BP_C* player = result.Value();
        frame.log.Info("Player health: {}", player->Health);
    }

    // Get world settings
    if (auto* settings = frame.world.GetWorldSettings()) {
        settings->TimeDilation = 0.5f;
    }

    // UI is automatically keybindable
    if (frame.ui.Button("Heal Player")) {
        // ...
    }
}
```

### When You Need Direct SDK Access

For advanced scenarios, you can still access the SDK directly, but **always from the game thread**:

```cpp
void OnFrame(Broadsword::Frame& frame) {
    // Direct SDK access - safe because OnFrame runs on game thread
    SDK::UWorld* World = SDK::UWorld::GetWorld();
    if (!World)
        return;

    SDK::APlayerController* Controller = SDK::UGameplayStatics::GetPlayerController(World, 0);
    if (!Controller || !Controller->Pawn)
        return;

    // Willie is a Blueprint class
    if (Controller->Pawn->IsA(SDK::AWillie_BP_C::StaticName())) {
        SDK::AWillie_BP_C* Willie = static_cast<SDK::AWillie_BP_C*>(Controller->Pawn);
        // Manipulate Willie here
    }
}
```

## Thread Safety - CRITICAL ⚠️

**All mod code runs on the game thread - you don't need to worry about thread safety!**

### Why This Works

Broadsword uses a unified thread model:
- **Game Thread**: Framework loop, mod lifecycle, SDK calls, ImGui frame building
- **Render Thread**: Only DirectX rendering (no mod code, no SDK calls)

### Safe Contexts

✅ `OnRegister(ModContext& ctx)` - Game thread
✅ `OnFrame(Frame& frame)` - Game thread (via event subscription)
✅ `OnUnregister()` - Game thread
✅ Hook callbacks - Game thread
✅ Event subscribers - Game thread

**You can safely call SDK functions in all of these contexts!**

## Common SDK Patterns

### 1. Getting the World

**Preferred (via Frame):**
```cpp
void OnFrame(Broadsword::Frame& frame) {
    auto result = frame.world.GetPlayer();
    // frame.world is a WorldFacade that handles errors
}
```

**Direct SDK (when needed):**
```cpp
SDK::UWorld* World = SDK::UWorld::GetWorld();
if (!World) {
    frame.log.Error("Failed to get UWorld");
    return;
}
```

**Never use `Offsets::GWorld` directly - always use `UWorld::GetWorld()`**

### 2. Getting the Player Controller

**Preferred:**
```cpp
auto result = frame.world.GetPlayer();
if (result) {
    SDK::AWillie_BP_C* player = result.Value();
}
```

**Direct SDK:**
```cpp
SDK::UWorld* World = SDK::UWorld::GetWorld();
SDK::APlayerController* Controller = SDK::UGameplayStatics::GetPlayerController(World, 0);
```

### 3. Finding Objects

```cpp
// Find by full name
SDK::UObject* Obj = SDK::UObject::FindObject("ClassName PackageName.Outer.ObjectName");

// Find by short name (faster)
SDK::UObject* Obj = SDK::UObject::FindObjectFast("ObjectName");

// Find a struct
SDK::UObject* Struct = SDK::UObject::FindObjectFast("StructName", EClassCastFlags::Struct);
```

### 4. Type Checking

```cpp
// Fast type check using cast flags (preferred)
if (Obj->IsA(EClassCastFlags::Actor)) {
    // Object is an AActor
}

// Type check using UClass (for native classes)
if (Obj->IsA(SDK::APlayerController::StaticClass())) {
    // Object is an APlayerController
}

// Type check using FName (for Blueprint classes)
if (Obj->IsA(SDK::AWillie_BP_C::StaticName())) {
    // Object is the Willie Blueprint class
}
```

### 5. Casting Objects

```cpp
if (Controller->Pawn && Controller->Pawn->IsA(SDK::AWillie_BP_C::StaticClass())) {
    SDK::AWillie_BP_C* Willie = static_cast<SDK::AWillie_BP_C*>(Controller->Pawn);
    // Safe to use Willie-specific members
}
```

### 6. Calling Functions

```cpp
// Instance method
SDK::APlayerController* Controller = GetController();
float MouseX, MouseY;
Controller->GetMousePosition(&MouseX, &MouseY);

// Static method (automatically uses DefaultObject)
SDK::FName MyName = SDK::UKismetStringLibrary::Conv_StringToName(L"SomeName");
```

## ProcessEvent Hooking

For advanced mods, you can hook any UE5 function:

```cpp
void OnRegister(Broadsword::ModContext& ctx) override {
    // Hook with typed parameters
    m_DamageHookId = ctx.hooks.Hook(
        "Function HalfSwordUE5.Willie_BP_C.TakeDamage",
        [this](SDK::UObject* obj, SDK::UFunction* func, void* params) -> bool {
            auto* damageParams = static_cast<SDK::FTakeDamageParams*>(params);

            if (m_GodMode) {
                return false;  // Block damage
            }

            damageParams->DamageAmount *= 0.5f;  // Reduce damage
            return true;  // Run original
        }
    );
}

void OnUnregister() override {
    ctx.hooks.Unhook(m_DamageHookId);
}
```

## Troubleshooting

### Build Error: `Cannot open include file: 'SDKPch.h'`

**Should not happen** - SDK include directory is pre-configured.

**Fix**: Verify CMakeLists.txt contains:
```cmake
target_include_directories(Broadsword
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Engine/SDK
)
```

### Linker Error: `Unresolved external symbol` for SDK functions

**Cause**: Missing `*_functions.cpp` file.

**Fix**: Should be auto-discovered. If not:
1. Verify the file exists in `Engine/SDK/SDK/`
2. Check CMakeLists.txt has: `file(GLOB_RECURSE SDK_SOURCES "Engine/SDK/SDK/*_functions.cpp")`
3. Clean rebuild: `cmake --build build --config Debug --clean-first`

### Linker Error: `Unresolved external symbol` for `InSDKUtils::GetImageBase()`

**Cause**: `Basic.cpp` not included.

**Fix**: Already included in CMakeLists.txt. If still failing, verify:
```cmake
# Engine - SDK
Engine/SDK/SDKPch.cpp
Engine/SDK/SDK/Basic.cpp
${SDK_SOURCES}
```

### SDK Functions Return `nullptr`

**Common causes**:
1. **Game not initialized** - Wait for first OnFrame event
2. **Wrong object name** - Use exact UE naming (e.g., `AWillie_BP_C` not `AWillie`)
3. **Object not spawned** - Check if the object exists in the world

### Crashes When Calling SDK Functions

**Should never happen in Broadsword!** All mod code runs on game thread.

If it does happen:
- Verify you're calling SDK functions inside mod lifecycle methods or event callbacks
- Check that you're not storing stale UObject pointers across frames

## Game-Specific Information

### Player Character Class

- **Class Name**: `SDK::AWillie_BP_C`
- **Type**: Blueprint class (ends with `_C`)
- **Codebase Name**: "Willie"

**Example**:
```cpp
void OnFrame(Broadsword::Frame& frame) {
    // Preferred: Use Frame API
    auto result = frame.world.GetPlayer();
    if (result) {
        SDK::AWillie_BP_C* Willie = result.Value();
        Willie->Health = 100.0f;
    }

    // Or direct SDK access
    SDK::UWorld* World = SDK::UWorld::GetWorld();
    SDK::APlayerController* Controller = SDK::UGameplayStatics::GetPlayerController(World, 0);
    if (Controller && Controller->Pawn) {
        if (Controller->Pawn->IsA(SDK::AWillie_BP_C::StaticName())) {
            SDK::AWillie_BP_C* Willie = static_cast<SDK::AWillie_BP_C*>(Controller->Pawn);
            // Willie-specific code
        }
    }
}
```

### Important Notes

1. **Blueprint classes end with `_C`**: `AWillie_BP_C`, `AMyActor_C`
2. **Always check for `nullptr`**: Validate all pointers before dereferencing
3. **Use Frame APIs when possible**: `frame.world`, `frame.ui`, `frame.log`
4. **Return Result<T>**: Prefer Result<T, Error> over raw pointers for error handling
5. **No manual memory management**: UE handles object lifetime automatically

## When to Regenerate SDK

Regenerate the SDK when:
- Game updates to a new version
- New game classes are added that you need to access
- Existing class structures change
- GWorld offset changes (check About window)

**How to regenerate**:
1. Use Dumper-7 on the updated game executable
2. Copy generated files to this directory
3. Update `Engine/SDK/SDKVersion.hpp` with new game version
4. Rebuild Broadsword

## Broadsword Philosophy

**Broadsword is designed to minimize direct SDK usage:**

- Use `frame.world` instead of `UWorld::GetWorld()`
- Use `frame.ui` for all UI (automatically keybindable)
- Use `frame.log` instead of raw logging
- Use `ctx.hooks` for ProcessEvent hooking
- Use `ctx.events` for game events

**The SDK is there when you need it, but the framework provides better alternatives for most use cases.**

## SDK Reference

For more detailed Dumper-7 SDK documentation:
- [Dumper-7 GitHub](https://github.com/Encryqed/Dumper-7)
- [Using Dumper-7 SDKs Guide](https://github.com/Encryqed/Dumper-7/blob/main/Dumper/Docs/using-the-sdk.md)

For Broadsword-specific documentation:
- `/docs/CREATING_MODS.md` - How to create mods
- `/docs/CONTRIBUTING.md` - Development guidelines
- `BROADSWORD_REDESIGN.md` - Complete framework architecture
