#include "GameFrameHook.hpp"
#include <algorithm>

namespace Broadsword::Foundation {

GameFrameHook* GameFrameHook::s_instance = nullptr;

GameFrameHook& GameFrameHook::Get() {
  static GameFrameHook instance;
  return instance;
}

GameFrameHook::~GameFrameHook() {
  Shutdown();
}

bool GameFrameHook::Initialize(void* viewportClient, size_t drawFunctionIndex) {
  if (m_initialized) {
    return false;  // Already initialized
  }

  if (!viewportClient) {
    return false;
  }

  m_drawFunctionIndex = drawFunctionIndex;
  s_instance = this;

  // Create VMT hook for viewport
  m_viewportHook = VTableHook::Create(viewportClient);
  if (!m_viewportHook) {
    return false;
  }

  // Hook the Draw function
  if (!m_viewportHook->Hook(drawFunctionIndex, reinterpret_cast<void*>(&OnDraw))) {
    return false;
  }

  // Apply the hook
  if (!m_viewportHook->Apply()) {
    return false;
  }

  // Record game thread ID (we're being called from game thread during init)
  m_gameThreadId = std::this_thread::get_id();

  m_lastFrameTime = std::chrono::steady_clock::now();
  m_initialized = true;

  return true;
}

void GameFrameHook::Shutdown() {
  if (!m_initialized) {
    return;
  }

  if (m_viewportHook) {
    m_viewportHook->Remove();
    m_viewportHook.reset();
  }

  m_initialized = false;
  s_instance = nullptr;
}

size_t GameFrameHook::RegisterFrameCallback(FrameCallback callback) {
  std::lock_guard<std::mutex> lock(m_callbacksMutex);

  size_t id = m_nextCallbackId++;
  m_callbacks.push_back({id, std::move(callback)});

  return id;
}

void GameFrameHook::UnregisterFrameCallback(size_t callbackId) {
  std::lock_guard<std::mutex> lock(m_callbacksMutex);

  m_callbacks.erase(std::remove_if(m_callbacks.begin(), m_callbacks.end(),
                                    [callbackId](const CallbackEntry& entry) { return entry.id == callbackId; }),
                    m_callbacks.end());
}

bool GameFrameHook::IsGameThread() const {
  return std::this_thread::get_id() == m_gameThreadId;
}

float GameFrameHook::GetFPS() const {
  // Calculate average FPS from samples
  float sum = 0.0f;
  size_t count = 0;

  for (size_t i = 0; i < FPS_SAMPLE_COUNT; i++) {
    if (m_fpsSamples[i] > 0.0f) {
      sum += m_fpsSamples[i];
      count++;
    }
  }

  if (count == 0) {
    return 0.0f;
  }

  return sum / count;
}

void GameFrameHook::QueueTask(std::function<void()> task) {
  std::lock_guard<std::mutex> lock(m_taskMutex);
  m_taskQueue.push_back(std::move(task));
}

void GameFrameHook::ProcessQueuedTasks() {
  // Swap queue to minimize lock time
  std::vector<std::function<void()>> localQueue;
  {
    std::lock_guard<std::mutex> lock(m_taskMutex);
    std::swap(localQueue, m_taskQueue);
  }

  // Execute tasks
  for (auto& task : localQueue) {
    try {
      task();
    } catch (...) {
      // Catch exceptions to prevent crashes
      // TODO: Log when logger is available
    }
  }
}

void GameFrameHook::OnDraw(void* viewportClient, void* viewport, void* canvas) {
  if (!s_instance || !s_instance->m_initialized) {
    return;
  }

  // Get original function and call it first (let game render)
  using DrawFunc = void (*)(void*, void*, void*);
  auto original = s_instance->m_viewportHook->GetOriginal<DrawFunc>(s_instance->m_drawFunctionIndex);
  if (original) {
    original(viewportClient, viewport, canvas);
  }

  // Now we're on game thread, game has rendered, perfect time for our code!

  // Calculate delta time
  auto now = std::chrono::steady_clock::now();
  auto deltaTimeDuration = std::chrono::duration<float>(now - s_instance->m_lastFrameTime);
  float deltaTime = deltaTimeDuration.count();
  s_instance->m_deltaTime = deltaTime;
  s_instance->m_lastFrameTime = now;

  // Update FPS sample
  if (deltaTime > 0.0f) {
    float fps = 1.0f / deltaTime;
    s_instance->m_fpsSamples[s_instance->m_fpsSampleIndex] = fps;
    s_instance->m_fpsSampleIndex = (s_instance->m_fpsSampleIndex + 1) % FPS_SAMPLE_COUNT;
  }

  // Increment frame counter
  s_instance->m_frameNumber++;

  // Process any queued tasks (fallback mechanism)
  s_instance->ProcessQueuedTasks();

  // Call all registered frame callbacks
  // Make a copy to avoid issues if callbacks modify the list
  std::vector<CallbackEntry> callbacks;
  {
    std::lock_guard<std::mutex> lock(s_instance->m_callbacksMutex);
    callbacks = s_instance->m_callbacks;
  }

  for (const auto& entry : callbacks) {
    try {
      entry.callback(deltaTime);
    } catch (...) {
      // Catch exceptions to prevent one bad callback from breaking everything
      // TODO: Log when logger is available
    }
  }

  // Note: ImGui rendering will happen in the framework's frame callback
  // which is registered as one of the callbacks above
}

// FrameTimer implementation

FrameTimer::FrameTimer() {
  m_lastFrameTime = std::chrono::steady_clock::now();
}

void FrameTimer::BeginFrame() {
  auto now = std::chrono::steady_clock::now();
  auto deltaTimeDuration = std::chrono::duration<float>(now - m_lastFrameTime);
  m_deltaTime = deltaTimeDuration.count();

  if (m_deltaTime > 0.0f) {
    m_fps = 1.0f / m_deltaTime;
  }

  m_lastFrameTime = now;
}

} // namespace Broadsword::Foundation
