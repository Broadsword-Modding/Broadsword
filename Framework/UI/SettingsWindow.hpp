#pragma once

#include <Windows.h>
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <string>

namespace Broadsword::Framework {

class SettingsWindow {
public:
    SettingsWindow();

    void Render();

    void SetVisible(bool visible) { m_Visible = visible; }
    bool IsVisible() const { return m_Visible; }

    void ToggleVisible() { m_Visible = !m_Visible; }

    void LoadFromConfig(const nlohmann::json& config);
    void SaveToConfig(nlohmann::json& config) const;

    // Get current menu toggle key
    int GetMenuToggleKey() const { return m_MenuToggleKey; }

private:
    void RenderGeneralSettings();
    void RenderThemeSettings();
    void RenderLoggingSettings();

    // Helper to render color picker for a single color
    bool RenderColorPicker(const char* label, ImVec4& color, const char* description = nullptr);

    // Helper to render a keybind selector
    bool RenderKeybindSelector(const char* label, int& keyCode, const char* description = nullptr);

    // Helper to convert virtual key code to display name
    static const char* GetKeyName(int vkCode);

    bool m_Visible = false;
    int m_SelectedTab = 0;

    // General settings
    int m_MenuToggleKey = VK_INSERT;
    bool m_ShowWelcomeNotification = true;
    bool m_SaveSettingsOnExit = true;

    // Logging settings
    int m_MinLogLevel = 2; // Info
    bool m_LogToConsole = true;
    bool m_LogToFile = true;
    bool m_LogToInGame = true;
    int m_MaxLogFiles = 5;
    float m_MaxLogFileSizeMB = 50.0f;

    // Keybind capture state
    bool m_CapturingKey = false;
    int m_CaptureTarget = 0;

    // Theme editing state
    bool m_ThemeModified = false;
};

} // namespace Broadsword::Framework
