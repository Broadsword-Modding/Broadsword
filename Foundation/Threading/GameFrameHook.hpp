#pragma once

#include "../Hooks/VTableHook.hpp"
#include <Windows.h>
#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

namespace Broadsword::Foundation {

/**
 * Game frame hook for synchronizing with the game's actual rendering.
 *
 * Broadsword uses a SINGLE-THREADED model:
 * - Hook UGameViewportClient::Draw (called every frame on game thread)
 * - All mod logic and UI runs in that hook (ONE callback!)
 * - No separate "render thread" callbacks
 * - No queuing needed (except as fallback for truly async operations)
 *
 * This eliminates the threading complexity from V1 and provides perfect
 * synchronization with the game's actual frame rate.
 *
 * Architecture:
 *   Game Thread:
 *     ├─ Game Tick
 *     ├─ UGameViewportClient::Draw ← WE HOOK HERE!
 *     │   ├─ Game renders world
 *     │   ├─ OnFrameCallbacks() ← Mods run here (game + UI together!)
 *     │   └─ ImGui rendering
 *     └─ Present (just swaps buffers)
 *
 * Variable FPS handling:
 * - 30 FPS game? We run at 30 FPS (correct - game state only updates 30x/sec)
 * - 144 FPS game? We run at 144 FPS (smooth UI!)
 * - FPS spikes? We experience them (correct - we're synced with game)
 */
class GameFrameHook {
public:
  using FrameCallback = std::function<void(float deltaTime)>;

  /**
   * Get the singleton instance.
   */
  static GameFrameHook& Get();

  // No copying or moving
  GameFrameHook(const GameFrameHook&) = delete;
  GameFrameHook& operator=(const GameFrameHook&) = delete;

  /**
   * Initialize the hook.
   * Must be called with a valid UGameViewportClient pointer from the SDK.
   *
   * @param viewportClient Pointer to UGameViewportClient (from SDK::UWorld::GameViewport)
   * @param drawFunctionIndex Index of Draw function in vtable (game-specific, usually 8-12)
   * @return true on success, false on failure
   */
  bool Initialize(void* viewportClient, size_t drawFunctionIndex);

  /**
   * Shutdown the hook and restore original function.
   */
  void Shutdown();

  /**
   * Check if hook is active.
   */
  bool IsInitialized() const { return m_initialized; }

  /**
   * Register a callback to be called every frame.
   * Callbacks run on the game thread during viewport drawing.
   *
   * @param callback Function to call each frame (receives deltaTime)
   * @return Callback ID (for unregistering)
   */
  size_t RegisterFrameCallback(FrameCallback callback);

  /**
   * Unregister a frame callback.
   *
   * @param callbackId ID returned by RegisterFrameCallback
   */
  void UnregisterFrameCallback(size_t callbackId);

  /**
   * Check if currently on the game thread.
   * Since we only have one thread (game thread), this should always be true
   * when called from framework/mod code.
   */
  bool IsGameThread() const;

  /**
   * Get the game thread ID.
   */
  std::thread::id GetGameThreadId() const { return m_gameThreadId; }

  /**
   * Get current frame number.
   */
  uint64_t GetFrameNumber() const { return m_frameNumber; }

  /**
   * Get delta time since last frame (seconds).
   */
  float GetDeltaTime() const { return m_deltaTime; }

  /**
   * Get current FPS (measured over last 60 frames).
   */
  float GetFPS() const;

  /**
   * Queue a task to run on next frame.
   * This is a FALLBACK for rare cases where you're on another thread.
   * Most mod code should just run directly in OnFrame() callback.
   *
   * @param task Function to execute next frame
   */
  void QueueTask(std::function<void()> task);

private:
  GameFrameHook() = default;
  ~GameFrameHook();

  // The actual hook function (called by VTableHook)
  static void OnDraw(void* viewportClient, void* viewport, void* canvas);

  // Process queued tasks (fallback for async operations)
  void ProcessQueuedTasks();

  std::unique_ptr<VTableHook> m_viewportHook;
  bool m_initialized = false;
  size_t m_drawFunctionIndex = 0;

  // Frame callbacks
  struct CallbackEntry {
    size_t id;
    FrameCallback callback;
  };
  std::vector<CallbackEntry> m_callbacks;
  std::mutex m_callbacksMutex;
  size_t m_nextCallbackId = 1;

  // Frame timing
  std::thread::id m_gameThreadId;
  std::atomic<uint64_t> m_frameNumber{0};
  std::chrono::steady_clock::time_point m_lastFrameTime;
  std::atomic<float> m_deltaTime{0.0f};

  // FPS measurement
  static constexpr size_t FPS_SAMPLE_COUNT = 60;
  float m_fpsSamples[FPS_SAMPLE_COUNT] = {};
  size_t m_fpsSampleIndex = 0;

  // Task queue (fallback only)
  std::vector<std::function<void()>> m_taskQueue;
  std::mutex m_taskMutex;

  // Static pointer for hook callback
  static GameFrameHook* s_instance;
};

/**
 * Frame timer utility for measuring delta time and FPS.
 * Can be used independently for timing measurements.
 */
class FrameTimer {
public:
  FrameTimer();

  /**
   * Mark the start of a new frame.
   */
  void BeginFrame();

  /**
   * Get delta time since last frame (seconds).
   */
  float GetDeltaTime() const { return m_deltaTime; }

  /**
   * Get current FPS (measured).
   */
  float GetFPS() const { return m_fps; }

private:
  std::chrono::steady_clock::time_point m_lastFrameTime;
  float m_deltaTime = 0.0f;
  float m_fps = 0.0f;
};

} // namespace Broadsword::Foundation
