#pragma once

#include <Windows.h>
#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

namespace Broadsword::Foundation {

/**
 * Game thread executor for safe UE5 SDK calls.
 *
 * Broadsword uses a unified thread model: ALL SDK calls must run on the game thread.
 * This eliminates the threading complexity that plagued V1.
 *
 * Architecture:
 * - Game Thread: Runs framework main loop at 60 FPS, processes queued tasks
 * - Render Thread: DirectX Present hook, renders ImGui (NO SDK CALLS)
 *
 * Usage from render thread:
 *   GameThreadExecutor::QueueTask([]() {
 *     SDK::UWorld* world = SDK::UWorld::GetWorld();  // Safe on game thread
 *     // ... SDK operations
 *   });
 *
 * The executor also provides precise frame timing to maintain 60 FPS.
 */
class GameThreadExecutor {
public:
  using Task = std::function<void()>;

  /**
   * Get the singleton instance.
   *
   * @return Reference to the executor
   */
  static GameThreadExecutor& Get();

  // No copying or moving
  GameThreadExecutor(const GameThreadExecutor&) = delete;
  GameThreadExecutor& operator=(const GameThreadExecutor&) = delete;

  /**
   * Start the game thread.
   * Creates a dedicated thread that runs at 60 FPS.
   *
   * @param targetFPS Target frames per second (default: 60)
   * @return true on success, false if already running
   */
  bool Start(double targetFPS = 60.0);

  /**
   * Stop the game thread.
   * Waits for all queued tasks to complete.
   */
  void Stop();

  /**
   * Check if the game thread is running.
   *
   * @return true if running, false otherwise
   */
  bool IsRunning() const { return m_running; }

  /**
   * Queue a task to run on the game thread.
   * Task will be executed on the next frame.
   *
   * @param task Function to execute
   */
  void QueueTask(Task task);

  /**
   * Execute all queued tasks (called by game thread loop).
   * Do not call this yourself unless you're implementing the main loop.
   *
   * @return Number of tasks executed
   */
  size_t ProcessQueue();

  /**
   * Check if currently on the game thread.
   *
   * @return true if on game thread, false otherwise
   */
  bool IsGameThread() const;

  /**
   * Get the game thread ID.
   *
   * @return Thread ID of the game thread
   */
  std::thread::id GetGameThreadId() const { return m_gameThreadId; }

  /**
   * Get current frame number.
   *
   * @return Frame count since start
   */
  uint64_t GetFrameNumber() const { return m_frameNumber; }

  /**
   * Get current frame time (time this frame started).
   *
   * @return Time point of current frame
   */
  std::chrono::steady_clock::time_point GetFrameTime() const { return m_frameStartTime; }

  /**
   * Get delta time since last frame.
   *
   * @return Delta time in seconds
   */
  double GetDeltaTime() const { return m_deltaTime; }

  /**
   * Get target FPS.
   *
   * @return Target frames per second
   */
  double GetTargetFPS() const { return m_targetFPS; }

  /**
   * Set target FPS.
   *
   * @param fps Target frames per second
   */
  void SetTargetFPS(double fps);

  /**
   * Get actual FPS (measured).
   *
   * @return Current frames per second
   */
  double GetActualFPS() const;

  /**
   * Wait for next frame (called by game thread loop).
   * Sleeps to maintain target FPS.
   */
  void WaitForNextFrame();

private:
  GameThreadExecutor() = default;
  ~GameThreadExecutor();

  // Game thread entry point
  void GameThreadLoop();

  std::thread m_gameThread;
  std::thread::id m_gameThreadId;
  std::atomic<bool> m_running{false};
  std::atomic<bool> m_stopRequested{false};

  // Task queue
  std::queue<Task> m_taskQueue;
  std::mutex m_queueMutex;

  // Frame timing
  std::atomic<uint64_t> m_frameNumber{0};
  std::chrono::steady_clock::time_point m_frameStartTime;
  std::chrono::steady_clock::time_point m_lastFrameTime;
  std::atomic<double> m_deltaTime{0.0};
  std::atomic<double> m_targetFPS{60.0};

  // FPS measurement
  static constexpr size_t FPS_SAMPLE_COUNT = 60;
  double m_fpsSamples[FPS_SAMPLE_COUNT] = {};
  size_t m_fpsSampleIndex = 0;
};

/**
 * Frame timer utility for maintaining precise frame timing.
 * Can be used independently of GameThreadExecutor.
 */
class FrameTimer {
public:
  explicit FrameTimer(double targetFPS);

  /**
   * Mark the start of a new frame.
   * Call this at the beginning of your frame loop.
   */
  void BeginFrame();

  /**
   * Wait until the next frame should start.
   * Sleeps to maintain target FPS.
   */
  void WaitForNextFrame();

  /**
   * Get delta time since last frame.
   *
   * @return Delta time in seconds
   */
  double GetDeltaTime() const { return m_deltaTime; }

  /**
   * Get target frame time.
   *
   * @return Target time per frame in seconds
   */
  double GetTargetFrameTime() const { return m_targetFrameTime; }

  /**
   * Set target FPS.
   *
   * @param fps Target frames per second
   */
  void SetTargetFPS(double fps);

private:
  double m_targetFPS;
  double m_targetFrameTime;  // 1.0 / targetFPS
  std::chrono::steady_clock::time_point m_frameStartTime;
  std::chrono::steady_clock::time_point m_lastFrameTime;
  double m_deltaTime;
};

} // namespace Broadsword::Foundation
