#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <memory>

namespace Broadsword {

/**
 * Generic event bus for publish-subscribe pattern
 *
 * Allows mods to subscribe to events and receive callbacks when those events are emitted.
 * Thread-safe for single-threaded usage (all callbacks execute on game thread).
 *
 * Usage:
 *   // Subscribe to an event
 *   size_t id = eventBus.Subscribe<OnFrameEvent>([](OnFrameEvent& e) {
 *       // Handle frame event
 *   });
 *
 *   // Emit an event
 *   OnFrameEvent event{frame};
 *   eventBus.Emit(event);
 *
 *   // Unsubscribe
 *   eventBus.Unsubscribe<OnFrameEvent>(id);
 */
class EventBus {
public:
    /**
     * Subscribe to an event type
     *
     * @param callback Function to call when event is emitted
     * @return Subscription ID for unsubscribing
     */
    template<typename Event>
    size_t Subscribe(std::function<void(Event&)> callback) {
        size_t id = m_NextId++;
        auto typeIndex = std::type_index(typeid(Event));

        // Create subscriber list if it doesn't exist
        if (m_Subscribers.find(typeIndex) == m_Subscribers.end()) {
            m_Subscribers[typeIndex] = std::make_unique<SubscriberList<Event>>();
        }

        // Add callback
        auto* subscribers = static_cast<SubscriberList<Event>*>(m_Subscribers[typeIndex].get());
        subscribers->callbacks[id] = callback;

        return id;
    }

    /**
     * Unsubscribe from an event type
     *
     * @param id Subscription ID returned from Subscribe
     */
    template<typename Event>
    void Unsubscribe(size_t id) {
        auto typeIndex = std::type_index(typeid(Event));

        auto it = m_Subscribers.find(typeIndex);
        if (it != m_Subscribers.end()) {
            auto* subscribers = static_cast<SubscriberList<Event>*>(it->second.get());
            subscribers->callbacks.erase(id);
        }
    }

    /**
     * Emit an event to all subscribers
     *
     * @param event Event instance to emit
     */
    template<typename Event>
    void Emit(Event& event) {
        auto typeIndex = std::type_index(typeid(Event));

        auto it = m_Subscribers.find(typeIndex);
        if (it != m_Subscribers.end()) {
            auto* subscribers = static_cast<SubscriberList<Event>*>(it->second.get());

            // Call all callbacks for this event type
            for (auto& [id, callback] : subscribers->callbacks) {
                callback(event);
            }
        }
    }

    /**
     * Get number of subscribers for an event type
     */
    template<typename Event>
    size_t GetSubscriberCount() const {
        auto typeIndex = std::type_index(typeid(Event));

        auto it = m_Subscribers.find(typeIndex);
        if (it != m_Subscribers.end()) {
            auto* subscribers = static_cast<SubscriberList<Event>*>(it->second.get());
            return subscribers->callbacks.size();
        }
        return 0;
    }

    /**
     * Clear all subscribers for all event types
     */
    void Clear() {
        m_Subscribers.clear();
    }

private:
    // Base class for type-erased subscriber storage
    struct ISubscriberList {
        virtual ~ISubscriberList() = default;
    };

    // Typed subscriber list
    template<typename Event>
    struct SubscriberList : ISubscriberList {
        std::unordered_map<size_t, std::function<void(Event&)>> callbacks;
    };

    // Type-erased subscriber storage
    std::unordered_map<std::type_index, std::unique_ptr<ISubscriberList>> m_Subscribers;

    // Subscription ID counter
    size_t m_NextId = 1;
};

} // namespace Broadsword
