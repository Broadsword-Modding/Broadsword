#include "NotificationManager.hpp"
#include "../../Services/UI/UIContext.hpp"

namespace Broadsword::Framework {

NotificationManager& NotificationManager::Get()
{
    static NotificationManager instance;
    return instance;
}

void NotificationManager::Render()
{
    const float PADDING = 10.0f;
    const float NOTIFICATION_WIDTH = 300.0f;
    const float NOTIFICATION_HEIGHT = 80.0f;

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 workPos = ImGui::GetMainViewport()->WorkPos;
    ImVec2 workSize = ImGui::GetMainViewport()->WorkSize;

    float yOffset = workSize.y - PADDING;

    auto now = std::chrono::steady_clock::now();

    // Render notifications from bottom to top
    for (auto it = m_Notifications.begin(); it != m_Notifications.end();)
    {
        auto& notif = *it;

        float elapsed =
            std::chrono::duration_cast<std::chrono::duration<float>>(now - notif.createdAt).count();

        // Remove expired notifications
        if (elapsed > notif.duration + notif.fadeOutDuration)
        {
            it = m_Notifications.erase(it);
            continue;
        }

        // Calculate alpha for fade-out effect
        float alpha = 1.0f;
        if (elapsed > notif.duration)
        {
            float fadeProgress = (elapsed - notif.duration) / notif.fadeOutDuration;
            alpha = 1.0f - fadeProgress;
        }

        // Position notification
        ImVec2 notifPos = ImVec2(workPos.x + workSize.x - NOTIFICATION_WIDTH - PADDING,
                                 workPos.y + yOffset - NOTIFICATION_HEIGHT);

        ImGui::SetNextWindowPos(notifPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(NOTIFICATION_WIDTH, NOTIFICATION_HEIGHT), ImGuiCond_Always);

        // Apply alpha to window background
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);

        char windowName[64];
        snprintf(windowName, sizeof(windowName), "##Notification%p", &notif);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                 ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                                 ImGuiWindowFlags_NoNav;

        if (ImGui::Begin(windowName, nullptr, flags))
        {
            const auto& theme = Services::UIContext::Get().GetTheme().GetColors();
            ImVec4 color = GetColorForType(notif.type);
            const char* icon = GetIconForType(notif.type);

            // Icon and title
            ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::Text("%s", icon);
            ImGui::PopStyleColor();

            ImGui::SameLine();
            ImGui::Text("%s", notif.title.c_str());

            // Message
            ImGui::Separator();
            ImGui::TextWrapped("%s", notif.message.c_str());

            // Progress bar
            float progress = elapsed / notif.duration;
            if (progress > 1.0f)
            {
                progress = 1.0f;
            }

            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, color);
            ImGui::ProgressBar(progress, ImVec2(-1, 2), "");
            ImGui::PopStyleColor();
        }
        ImGui::End();

        ImGui::PopStyleVar();

        yOffset -= NOTIFICATION_HEIGHT + PADDING;
        ++it;
    }
}

void NotificationManager::AddNotification(NotificationType type, const std::string& title,
                                           const std::string& message, float duration)
{
    m_Notifications.push_back(Notification{
        .type = type,
        .title = title,
        .message = message,
        .createdAt = std::chrono::steady_clock::now(),
        .duration = duration,
        .fadeOutDuration = 0.3f,
    });
}

void NotificationManager::Info(const std::string& title, const std::string& message, float duration)
{
    AddNotification(NotificationType::Info, title, message, duration);
}

void NotificationManager::Success(const std::string& title, const std::string& message, float duration)
{
    AddNotification(NotificationType::Success, title, message, duration);
}

void NotificationManager::Warning(const std::string& title, const std::string& message, float duration)
{
    AddNotification(NotificationType::Warning, title, message, duration);
}

void NotificationManager::Error(const std::string& title, const std::string& message, float duration)
{
    AddNotification(NotificationType::Error, title, message, duration);
}

void NotificationManager::Clear()
{
    m_Notifications.clear();
}

ImVec4 NotificationManager::GetColorForType(NotificationType type) const
{
    const auto& theme = Services::UIContext::Get().GetTheme().GetColors();

    switch (type)
    {
    case NotificationType::Info:
        return theme.Info();
    case NotificationType::Success:
        return theme.Success();
    case NotificationType::Warning:
        return theme.Warning();
    case NotificationType::Error:
        return theme.Error();
    default:
        return theme.foreground;
    }
}

const char* NotificationManager::GetIconForType(NotificationType type) const
{
    switch (type)
    {
    case NotificationType::Info:
        return "[i]";
    case NotificationType::Success:
        return "[OK]";
    case NotificationType::Warning:
        return "[!]";
    case NotificationType::Error:
        return "[X]";
    default:
        return "[*]";
    }
}

} // namespace Broadsword::Framework
