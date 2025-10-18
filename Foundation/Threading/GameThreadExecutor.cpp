#include "GameThreadExecutor.hpp"

namespace Broadsword::Foundation
{

GameThreadExecutor& GameThreadExecutor::Get()
{
    static GameThreadExecutor instance;
    return instance;
}

void GameThreadExecutor::QueueAction(std::function<void()> action)
{
    if (action)
    {
        m_ActionQueue.push(std::move(action));
    }
}

void GameThreadExecutor::ProcessQueue()
{
    // Process all queued actions
    // This runs in the Present hook at the game's natural FPS
    while (!m_ActionQueue.empty())
    {
        auto action = std::move(m_ActionQueue.front());
        m_ActionQueue.pop();

        // Execute the action
        // If it throws, we'll catch it in the caller (Framework main loop)
        action();
    }
}

void GameThreadExecutor::Clear()
{
    // Clear all pending actions
    while (!m_ActionQueue.empty())
    {
        m_ActionQueue.pop();
    }
}

size_t GameThreadExecutor::PendingCount() const
{
    return m_ActionQueue.size();
}

} // namespace Broadsword::Foundation
