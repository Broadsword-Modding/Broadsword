#pragma once

#include "../../Services/UI/Theme.hpp"
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace Broadsword::Framework {

enum class LogLevel {
    Trace,
    Debug,
    Info,
    Warning,
    Error,
    Critical,
};

struct ConsoleMessage {
    LogLevel level;
    std::string message;
    std::string timestamp;
};

class ConsoleWindow {
public:
    ConsoleWindow();

    void Render();

    void AddMessage(LogLevel level, const std::string& message);

    void Clear();

    void SetAutoScroll(bool enabled) { m_AutoScroll = enabled; }
    bool GetAutoScroll() const { return m_AutoScroll; }

    void SetVisible(bool visible) { m_Visible = visible; }
    bool IsVisible() const { return m_Visible; }

    void LoadFromConfig(const nlohmann::json& config);
    void SaveToConfig(nlohmann::json& config) const;

private:
    ImVec4 GetColorForLogLevel(LogLevel level) const;
    const char* GetIconForLogLevel(LogLevel level) const;

    std::vector<ConsoleMessage> m_Messages;
    bool m_AutoScroll = true;
    bool m_Visible = true;  // Open by default

    bool m_ShowTrace = false;
    bool m_ShowDebug = true;  // Debug and above by default
    bool m_ShowInfo = true;
    bool m_ShowWarning = true;
    bool m_ShowError = true;
    bool m_ShowCritical = true;

    char m_SearchBuffer[256] = {0};
    std::string m_TextBuffer;
};

} // namespace Broadsword::Framework
