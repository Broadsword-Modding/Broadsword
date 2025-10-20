#include "ConsoleWindow.hpp"
#include "../../Services/Logging/Logger.hpp"
#include "../../Services/UI/UIContext.hpp"
#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace Broadsword::Framework {

ConsoleWindow::ConsoleWindow()
{
    m_Messages.reserve(1000);
}

void ConsoleWindow::Render()
{
    if (!m_Visible)
    {
        return;
    }

    // Pin to bottom of screen, full width
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 workPos = viewport->WorkPos;
    ImVec2 workSize = viewport->WorkSize;

    const float consoleHeight = 300.0f;
    ImGui::SetNextWindowPos(ImVec2(workPos.x, workPos.y + workSize.y - consoleHeight), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(workSize.x, consoleHeight), ImGuiCond_Always);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

    if (!ImGui::Begin("Console", &m_Visible, windowFlags))
    {
        ImGui::End();
        return;
    }

    // Toolbar
    if (ImGui::BeginChild("ConsoleToolbar", ImVec2(0, 40), true))
    {
        // Search bar
        ImGui::SetNextItemWidth(250.0f);
        ImGui::InputTextWithHint("##ConsoleSearch", "Search...", m_SearchBuffer, sizeof(m_SearchBuffer));

        ImGui::SameLine();

        // Log level filters
        ImGui::Checkbox("Trace", &m_ShowTrace);
        ImGui::SameLine();
        ImGui::Checkbox("Debug", &m_ShowDebug);
        ImGui::SameLine();
        ImGui::Checkbox("Info", &m_ShowInfo);
        ImGui::SameLine();
        ImGui::Checkbox("Warning", &m_ShowWarning);
        ImGui::SameLine();
        ImGui::Checkbox("Error", &m_ShowError);
        ImGui::SameLine();
        ImGui::Checkbox("Critical", &m_ShowCritical);

        ImGui::SameLine();
        ImGui::Spacing();
        ImGui::SameLine();

        // Auto-scroll toggle
        ImGui::Checkbox("Auto-scroll", &m_AutoScroll);

        ImGui::SameLine();

        // Clear button
        if (ImGui::Button("Clear"))
        {
            Clear();
        }
    }
    ImGui::EndChild();

    // Build text buffer from filtered logs
    m_TextBuffer.clear();
    m_TextBuffer.reserve(100000);
    const std::string searchStr = m_SearchBuffer;

    // Query logs from Logger (limit to last 500 for performance)
    auto logs = Services::Logger::Get().QueryLogs(
        std::nullopt,  // min_level
        std::nullopt,  // mod_filter
        std::nullopt,  // frame_start
        std::nullopt,  // frame_end
        500            // max_results
    );

    // Reverse iteration - newest logs first (at top)
    for (auto it = logs.rbegin(); it != logs.rend(); ++it)
    {
        const auto& entry = *it;

        // Convert Services::LogLevel to Framework::LogLevel for filtering
        Framework::LogLevel level;
        switch (entry.level)
        {
        case Services::LogLevel::Trace:
            level = Framework::LogLevel::Trace;
            break;
        case Services::LogLevel::Debug:
            level = Framework::LogLevel::Debug;
            break;
        case Services::LogLevel::Info:
            level = Framework::LogLevel::Info;
            break;
        case Services::LogLevel::Warning:
            level = Framework::LogLevel::Warning;
            break;
        case Services::LogLevel::Error:
            level = Framework::LogLevel::Error;
            break;
        case Services::LogLevel::Critical:
            level = Framework::LogLevel::Critical;
            break;
        }

        // Apply log level filter
        bool show = false;
        switch (level)
        {
        case Framework::LogLevel::Trace:
            show = m_ShowTrace;
            break;
        case Framework::LogLevel::Debug:
            show = m_ShowDebug;
            break;
        case Framework::LogLevel::Info:
            show = m_ShowInfo;
            break;
        case Framework::LogLevel::Warning:
            show = m_ShowWarning;
            break;
        case Framework::LogLevel::Error:
            show = m_ShowError;
            break;
        case Framework::LogLevel::Critical:
            show = m_ShowCritical;
            break;
        }

        if (!show)
        {
            continue;
        }

        // Apply search filter
        if (!searchStr.empty())
        {
            std::string msgLower = entry.message;
            std::transform(msgLower.begin(), msgLower.end(), msgLower.begin(), ::tolower);

            std::string searchLower = searchStr;
            std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);

            if (msgLower.find(searchLower) == std::string::npos)
            {
                continue;
            }
        }

        // Format timestamp
        auto t = std::chrono::system_clock::to_time_t(entry.timestamp);
        auto ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(entry.timestamp.time_since_epoch()) % 1000;
        std::tm localTime;
        localtime_s(&localTime, &t);
        char timeBuffer[32];
        std::snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d:%02d.%03d", localTime.tm_hour, localTime.tm_min,
                      localTime.tm_sec, static_cast<int>(ms.count()));

        // Build full log line
        const char* icon = GetIconForLogLevel(level);
        m_TextBuffer += "[" + std::string(timeBuffer) + "] ";
        m_TextBuffer += "[F:" + std::to_string(entry.frame_number) + "] ";
        m_TextBuffer += std::string(icon) + " ";
        if (!entry.context.mod_name.empty())
        {
            m_TextBuffer += "[" + entry.context.mod_name + "] ";
        }
        m_TextBuffer += entry.message;
        m_TextBuffer += "\n";
    }

    ImGuiInputTextFlags flags = ImGuiInputTextFlags_ReadOnly;

    ImGui::InputTextMultiline("##ConsoleText", m_TextBuffer.data(), m_TextBuffer.size() + 1,
                               ImVec2(-1, -1), flags);

    ImGui::End();
}

void ConsoleWindow::AddMessage(LogLevel level, const std::string& message)
{
    // Generate timestamp
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::tm localTime;
    localtime_s(&localTime, &time);

    std::ostringstream oss;
    oss << std::put_time(&localTime, "%H:%M:%S") << "." << std::setfill('0') << std::setw(3) << ms.count();

    m_Messages.push_back(ConsoleMessage{
        .level = level,
        .message = message,
        .timestamp = oss.str(),
    });

    // Limit message history to prevent memory issues
    constexpr size_t MAX_MESSAGES = 10000;
    if (m_Messages.size() > MAX_MESSAGES)
    {
        m_Messages.erase(m_Messages.begin(), m_Messages.begin() + (m_Messages.size() - MAX_MESSAGES));
    }
}

void ConsoleWindow::Clear()
{
    m_Messages.clear();
}

ImVec4 ConsoleWindow::GetColorForLogLevel(LogLevel level) const
{
    const auto& theme = Services::UIContext::Get().GetTheme().GetColors();

    switch (level)
    {
    case LogLevel::Trace:
        return theme.normal.cyan;
    case LogLevel::Debug:
        return theme.normal.blue;
    case LogLevel::Info:
        return theme.normal.green;
    case LogLevel::Warning:
        return theme.normal.yellow;
    case LogLevel::Error:
        return theme.normal.red;
    case LogLevel::Critical:
        return theme.bright.red;
    default:
        return theme.foreground;
    }
}

const char* ConsoleWindow::GetIconForLogLevel(LogLevel level) const
{
    switch (level)
    {
    case LogLevel::Trace:
        return "[TRACE]";
    case LogLevel::Debug:
        return "[DEBUG]";
    case LogLevel::Info:
        return "[INFO ]";
    case LogLevel::Warning:
        return "[WARN ]";
    case LogLevel::Error:
        return "[ERROR]";
    case LogLevel::Critical:
        return "[CRIT ]";
    default:
        return "[LOG  ]";
    }
}

void ConsoleWindow::LoadFromConfig(const nlohmann::json& config)
{
    if (!config.contains("console"))
    {
        return;
    }

    const auto& consoleConfig = config["console"];

    if (consoleConfig.contains("auto_scroll"))
    {
        m_AutoScroll = consoleConfig["auto_scroll"].get<bool>();
    }

    if (consoleConfig.contains("show_trace"))
    {
        m_ShowTrace = consoleConfig["show_trace"].get<bool>();
    }

    if (consoleConfig.contains("show_debug"))
    {
        m_ShowDebug = consoleConfig["show_debug"].get<bool>();
    }

    if (consoleConfig.contains("show_info"))
    {
        m_ShowInfo = consoleConfig["show_info"].get<bool>();
    }

    if (consoleConfig.contains("show_warning"))
    {
        m_ShowWarning = consoleConfig["show_warning"].get<bool>();
    }

    if (consoleConfig.contains("show_error"))
    {
        m_ShowError = consoleConfig["show_error"].get<bool>();
    }

    if (consoleConfig.contains("show_critical"))
    {
        m_ShowCritical = consoleConfig["show_critical"].get<bool>();
    }
}

void ConsoleWindow::SaveToConfig(nlohmann::json& config) const
{
    auto& consoleConfig = config["console"];

    consoleConfig["auto_scroll"] = m_AutoScroll;
    consoleConfig["show_trace"] = m_ShowTrace;
    consoleConfig["show_debug"] = m_ShowDebug;
    consoleConfig["show_info"] = m_ShowInfo;
    consoleConfig["show_warning"] = m_ShowWarning;
    consoleConfig["show_error"] = m_ShowError;
    consoleConfig["show_critical"] = m_ShowCritical;
}

} // namespace Broadsword::Framework
