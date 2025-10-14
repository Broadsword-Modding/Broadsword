#pragma once

#include <Windows.h>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <unordered_map>

namespace Broadsword::Foundation {

/**
 * VMT (Virtual Method Table) hooking utility.
 *
 * Used for hooking virtual functions in C++ classes (e.g., DirectX interfaces, UE5 UObject).
 * Safer than inline hooking for virtual methods because:
 * - No code modification (just pointer swaps)
 * - Easy to chain multiple hooks
 * - Can be done per-instance (not global)
 *
 * Usage:
 *   auto hook = VTableHook::Create(pObject);
 *   hook->Hook(3, MyPresent); // Hook IDXGISwapChain::Present (index 8)
 *   hook->Apply();
 *
 *   Later: hook->Unhook(3); or hook->Remove();
 */
class VTableHook {
public:
  using HookCallback = void*;

  /**
   * Create a VTableHook for an object.
   *
   * @param pObject Pointer to the object whose vtable to hook
   * @return Unique pointer to VTableHook, or nullptr on failure
   */
  static std::unique_ptr<VTableHook> Create(void* pObject);

  /**
   * Destructor - automatically removes all hooks.
   */
  ~VTableHook();

  // No copying or moving (vtable hooks are tied to specific object instances)
  VTableHook(const VTableHook&) = delete;
  VTableHook& operator=(const VTableHook&) = delete;
  VTableHook(VTableHook&&) = delete;
  VTableHook& operator=(VTableHook&&) = delete;

  /**
   * Hook a virtual function at a specific index.
   *
   * @param index Index in the vtable (0-based)
   * @param hookFunction Pointer to your hook function (must have same signature as original)
   * @return true on success, false on failure
   */
  bool Hook(size_t index, void* hookFunction);

  /**
   * Unhook a virtual function at a specific index.
   *
   * @param index Index in the vtable (0-based)
   * @return true on success, false on failure
   */
  bool Unhook(size_t index);

  /**
   * Get the original function pointer for a hooked index.
   * Use this to call the original function from your hook.
   *
   * @param index Index in the vtable (0-based)
   * @return Pointer to the original function, or nullptr if not hooked
   */
  void* GetOriginal(size_t index) const;

  /**
   * Get the original function pointer cast to a specific type.
   * Convenience template for type safety.
   *
   * @tparam T Function pointer type
   * @param index Index in the vtable (0-based)
   * @return Pointer to the original function, or nullptr if not hooked
   */
  template <typename T>
  T GetOriginal(size_t index) const {
    return reinterpret_cast<T>(GetOriginal(index));
  }

  /**
   * Apply all queued hooks.
   * Hooks are not active until this is called.
   *
   * @return true on success, false on failure
   */
  bool Apply();

  /**
   * Remove all hooks and restore original vtable.
   */
  void Remove();

  /**
   * Check if a specific index is hooked.
   *
   * @param index Index in the vtable (0-based)
   * @return true if hooked, false otherwise
   */
  bool IsHooked(size_t index) const;

  /**
   * Get the vtable size (number of virtual functions).
   *
   * @return Number of entries in the vtable
   */
  size_t GetVTableSize() const;

private:
  VTableHook(void* pObject, void** originalVTable, size_t vtableSize);

  void* m_pObject;                   // Object being hooked
  void** m_pOriginalVTable;          // Original vtable
  void** m_pNewVTable;               // Our modified vtable
  size_t m_vtableSize;               // Number of functions in vtable
  bool m_applied;                    // Whether hooks are active

  std::unordered_map<size_t, void*> m_hooks;  // index -> hook function
};

/**
 * RAII wrapper for VTableHook.
 * Automatically applies hooks on construction and removes on destruction.
 */
class ScopedVTableHook {
public:
  ScopedVTableHook(void* pObject) : m_hook(VTableHook::Create(pObject)) {}

  ~ScopedVTableHook() {
    if (m_hook) {
      m_hook->Remove();
    }
  }

  VTableHook* operator->() { return m_hook.get(); }
  const VTableHook* operator->() const { return m_hook.get(); }

  VTableHook& Get() { return *m_hook; }
  const VTableHook& Get() const { return *m_hook; }

private:
  std::unique_ptr<VTableHook> m_hook;
};

} // namespace Broadsword::Foundation
