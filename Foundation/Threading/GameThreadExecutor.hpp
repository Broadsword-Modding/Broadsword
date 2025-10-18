#pragma once

#include <functional>
#include <queue>

namespace Broadsword::Foundation
{

// GameThreadExecutor - Queues actions to be executed on the game thread
//
// The game thread is the only thread safe for UE5 SDK calls.
// This executor runs in the DirectX Present hook, which is called at the game's natural FPS.
//
// NO frame timing, NO sleep, NO hardcoded FPS - just process queue when game renders.
class GameThreadExecutor
{
public:
    static GameThreadExecutor& Get();

    // Queue an action to be executed on the next frame
    void QueueAction(std::function<void()> action);

    // Process all queued actions
    // Called from the Present hook (runs at game's FPS)
    void ProcessQueue();

    // Clear all pending actions
    void Clear();

    // Get number of pending actions
    size_t PendingCount() const;

private:
    GameThreadExecutor() = default;
    ~GameThreadExecutor() = default;

    // Delete copy/move
    GameThreadExecutor(const GameThreadExecutor&) = delete;
    GameThreadExecutor& operator=(const GameThreadExecutor&) = delete;
    GameThreadExecutor(GameThreadExecutor&&) = delete;
    GameThreadExecutor& operator=(GameThreadExecutor&&) = delete;

    std::queue<std::function<void()>> m_ActionQueue;

    // NOTE: No mutex needed - ProcessQueue() is only called from Present hook (game thread)
    // and mods should only queue actions from game thread (OnFrame callbacks)
};

} // namespace Broadsword::Foundation
