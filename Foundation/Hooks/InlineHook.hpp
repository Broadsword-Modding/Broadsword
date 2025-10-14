#pragma once

#include <MinHook.h>
#include <Windows.h>
#include <cstdint>
#include <string>

namespace Broadsword::Foundation {

/**
 * Inline hooking utility using MinHook.
 *
 * Inline hooks modify the beginning of a function's code to jump to your hook.
 * More invasive than VMT hooks, but works for non-virtual functions.
 *
 * MinHook handles:
 * - Instruction relocation
 * - Thread safety
 * - Trampoline generation
 *
 * Usage:
 *   InlineHook::Initialize();
 *   InlineHook hook(targetFunction, myHook);
 *   hook.Enable();
 *   // ... call originalFunction via hook.GetOriginal<T>()
 *   hook.Disable();
 *   InlineHook::Shutdown();
 */
class InlineHook {
public:
  /**
   * Initialize MinHook library.
   * Must be called once before creating any hooks.
   *
   * @return true on success, false on failure
   */
  static bool Initialize();

  /**
   * Shutdown MinHook library.
   * Call this when done with all hooks (e.g., on framework shutdown).
   */
  static void Shutdown();

  /**
   * Create an inline hook.
   *
   * @param pTarget Pointer to the function to hook
   * @param pDetour Pointer to your hook function (must have same signature as target)
   */
  InlineHook(void* pTarget, void* pDetour);

  /**
   * Destructor - automatically disables and removes the hook.
   */
  ~InlineHook();

  // No copying (hooks are unique resources)
  InlineHook(const InlineHook&) = delete;
  InlineHook& operator=(const InlineHook&) = delete;

  // Moving is allowed
  InlineHook(InlineHook&& other) noexcept;
  InlineHook& operator=(InlineHook&& other) noexcept;

  /**
   * Enable the hook.
   *
   * @return true on success, false on failure
   */
  bool Enable();

  /**
   * Disable the hook (restore original function).
   *
   * @return true on success, false on failure
   */
  bool Disable();

  /**
   * Check if hook is enabled.
   *
   * @return true if enabled, false otherwise
   */
  bool IsEnabled() const { return m_enabled; }

  /**
   * Get pointer to the original function (trampoline).
   * Use this to call the original function from your hook.
   *
   * @return Pointer to original function
   */
  void* GetOriginal() const { return m_pOriginal; }

  /**
   * Get pointer to the original function, cast to specific type.
   *
   * @tparam T Function pointer type
   * @return Typed pointer to original function
   */
  template <typename T>
  T GetOriginal() const {
    return reinterpret_cast<T>(m_pOriginal);
  }

  /**
   * Get the target function pointer.
   *
   * @return Pointer to target function
   */
  void* GetTarget() const { return m_pTarget; }

  /**
   * Get the detour function pointer.
   *
   * @return Pointer to detour function
   */
  void* GetDetour() const { return m_pDetour; }

  /**
   * Get a human-readable error message for a MinHook status code.
   *
   * @param status MinHook status code
   * @return Error message string
   */
  static std::string GetErrorString(MH_STATUS status);

private:
  void* m_pTarget;    // Original function to hook
  void* m_pDetour;    // Our hook function
  void* m_pOriginal;  // Trampoline to original function
  bool m_enabled;     // Whether hook is active
  bool m_created;     // Whether hook was created successfully
};

/**
 * RAII wrapper for InlineHook.
 * Automatically enables on construction and disables on destruction.
 */
class ScopedInlineHook {
public:
  ScopedInlineHook(void* pTarget, void* pDetour) : m_hook(pTarget, pDetour) { m_hook.Enable(); }

  ~ScopedInlineHook() { m_hook.Disable(); }

  InlineHook& Get() { return m_hook; }
  const InlineHook& Get() const { return m_hook; }

  template <typename T>
  T GetOriginal() const {
    return m_hook.GetOriginal<T>();
  }

private:
  InlineHook m_hook;
};

} // namespace Broadsword::Foundation
