#include "GameThreadExecutor.hpp"
#include <algorithm>

namespace Broadsword::Foundation {

GameThreadExecutor& GameThreadExecutor::Get() {
  static GameThreadExecutor instance;
  return instance;
}

GameThreadExecutor::~GameThreadExecutor() {
  Stop();
}

bool GameThreadExecutor::Start(double targetFPS) {
  if (m_running) {
    return false;
  }

  m_targetFPS = targetFPS;
  m_stopRequested = false;
  m_running = true;
  m_frameNumber = 0;

  // Create game thread
  m_gameThread = std::thread([this]() { GameThreadLoop(); });
  m_gameThreadId = m_gameThread.get_id();

  return true;
}

void GameThreadExecutor::Stop() {
  if (!m_running) {
    return;
  }

  m_stopRequested = true;

  // Wait for game thread to finish
  if (m_gameThread.joinable()) {
    m_gameThread.join();
  }

  m_running = false;
}

void GameThreadExecutor::QueueTask(Task task) {
  std::lock_guard<std::mutex> lock(m_queueMutex);
  m_taskQueue.push(std::move(task));
}

size_t GameThreadExecutor::ProcessQueue() {
  // Swap queue to minimize lock time
  std::queue<Task> localQueue;
  {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    std::swap(localQueue, m_taskQueue);
  }

  size_t count = 0;
  while (!localQueue.empty()) {
    try {
      localQueue.front()();
      count++;
    } catch (...) {
      // Catch exceptions to prevent one bad task from crashing the game thread
      // TODO: Log exception once logging system is implemented
    }
    localQueue.pop();
  }

  return count;
}

bool GameThreadExecutor::IsGameThread() const {
  return std::this_thread::get_id() == m_gameThreadId;
}

void GameThreadExecutor::SetTargetFPS(double fps) {
  m_targetFPS = std::max(1.0, fps);
}

double GameThreadExecutor::GetActualFPS() const {
  // Calculate average FPS from samples
  double sum = 0.0;
  size_t count = 0;

  for (size_t i = 0; i < FPS_SAMPLE_COUNT; i++) {
    if (m_fpsSamples[i] > 0.0) {
      sum += m_fpsSamples[i];
      count++;
    }
  }

  if (count == 0) {
    return 0.0;
  }

  return sum / count;
}

void GameThreadExecutor::WaitForNextFrame() {
  auto now = std::chrono::steady_clock::now();
  double targetFrameTime = 1.0 / m_targetFPS.load();
  auto targetDuration = std::chrono::duration<double>(targetFrameTime);

  auto elapsed = std::chrono::duration<double>(now - m_frameStartTime).count();
  double sleepTime = targetFrameTime - elapsed;

  if (sleepTime > 0.0) {
    // Sleep for most of the remaining time
    auto sleepDuration = std::chrono::duration<double>(sleepTime * 0.95);  // Sleep 95% to avoid oversleep
    std::this_thread::sleep_for(sleepDuration);

    // Busy-wait for the remaining time (more precise)
    while (std::chrono::steady_clock::now() - m_frameStartTime < targetDuration) {
      std::this_thread::yield();
    }
  }
}

void GameThreadExecutor::GameThreadLoop() {
  m_frameStartTime = std::chrono::steady_clock::now();
  m_lastFrameTime = m_frameStartTime;

  while (!m_stopRequested) {
    // Mark frame start
    m_frameStartTime = std::chrono::steady_clock::now();

    // Calculate delta time
    auto deltaTimeDuration = std::chrono::duration<double>(m_frameStartTime - m_lastFrameTime);
    m_deltaTime = deltaTimeDuration.count();

    // Update FPS sample
    if (m_deltaTime > 0.0) {
      double fps = 1.0 / m_deltaTime;
      m_fpsSamples[m_fpsSampleIndex] = fps;
      m_fpsSampleIndex = (m_fpsSampleIndex + 1) % FPS_SAMPLE_COUNT;
    }

    // Process queued tasks
    ProcessQueue();

    // Increment frame counter
    m_frameNumber++;

    // Wait for next frame
    WaitForNextFrame();

    m_lastFrameTime = m_frameStartTime;
  }
}

// FrameTimer implementation

FrameTimer::FrameTimer(double targetFPS)
    : m_targetFPS(targetFPS), m_targetFrameTime(1.0 / targetFPS), m_deltaTime(0.0) {
  m_frameStartTime = std::chrono::steady_clock::now();
  m_lastFrameTime = m_frameStartTime;
}

void FrameTimer::BeginFrame() {
  m_frameStartTime = std::chrono::steady_clock::now();
  auto deltaTimeDuration = std::chrono::duration<double>(m_frameStartTime - m_lastFrameTime);
  m_deltaTime = deltaTimeDuration.count();
  m_lastFrameTime = m_frameStartTime;
}

void FrameTimer::WaitForNextFrame() {
  auto now = std::chrono::steady_clock::now();
  auto targetDuration = std::chrono::duration<double>(m_targetFrameTime);

  auto elapsed = std::chrono::duration<double>(now - m_frameStartTime).count();
  double sleepTime = m_targetFrameTime - elapsed;

  if (sleepTime > 0.0) {
    // Sleep for most of the remaining time
    auto sleepDuration = std::chrono::duration<double>(sleepTime * 0.95);
    std::this_thread::sleep_for(sleepDuration);

    // Busy-wait for the remaining time
    while (std::chrono::steady_clock::now() - m_frameStartTime < targetDuration) {
      std::this_thread::yield();
    }
  }
}

void FrameTimer::SetTargetFPS(double fps) {
  m_targetFPS = std::max(1.0, fps);
  m_targetFrameTime = 1.0 / m_targetFPS;
}

} // namespace Broadsword::Foundation
