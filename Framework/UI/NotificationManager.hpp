#pragma once

#include <chrono>
#include <imgui.h>
#include <string>
#include <vector>

namespace Broadsword::Framework {

enum class NotificationType {
    Info,
    Success,
    Warning,
    Error,
};

struct Notification {
    NotificationType type;
    std::string title;
    std::string message;
    std::chrono::steady_clock::time_point createdAt;
    float duration;
    float fadeOutDuration = 0.3f;
};

class NotificationManager {
public:
    static NotificationManager& Get();

    NotificationManager(const NotificationManager&) = delete;
    NotificationManager& operator=(const NotificationManager&) = delete;

    void Render();

    void AddNotification(NotificationType type, const std::string& title, const std::string& message,
                         float duration = 3.0f);

    void Info(const std::string& title, const std::string& message, float duration = 3.0f);
    void Success(const std::string& title, const std::string& message, float duration = 3.0f);
    void Warning(const std::string& title, const std::string& message, float duration = 3.0f);
    void Error(const std::string& title, const std::string& message, float duration = 5.0f);

    void Clear();

private:
    NotificationManager() = default;

    ImVec4 GetColorForType(NotificationType type) const;
    const char* GetIconForType(NotificationType type) const;

    std::vector<Notification> m_Notifications;
};

} // namespace Broadsword::Framework
