#include "SettingsWindow.hpp"
#include "../../Services/Logging/Logger.hpp"
#include "../../Services/UI/UIContext.hpp"
#include <algorithm>
#include <Windows.h>

namespace Broadsword::Framework {

SettingsWindow::SettingsWindow()
{
}

void SettingsWindow::Render()
{
    if (!m_Visible)
    {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Broadsword Settings", &m_Visible))
    {
        ImGui::End();
        return;
    }

    // Tab bar for different settings categories
    if (ImGui::BeginTabBar("SettingsTabs", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("General"))
        {
            m_SelectedTab = 0;
            RenderGeneralSettings();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Theme"))
        {
            m_SelectedTab = 1;
            RenderThemeSettings();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Logging"))
        {
            m_SelectedTab = 2;
            RenderLoggingSettings();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void SettingsWindow::RenderGeneralSettings()
{
    ImGui::BeginChild("GeneralSettings", ImVec2(0, 0), false);

    ImGui::SeparatorText("Interface");

    // Menu toggle key
    RenderKeybindSelector("Menu Toggle Key", m_MenuToggleKey, "Key to open/close the Broadsword menu");

    ImGui::Spacing();

    // Checkboxes
    if (ImGui::Checkbox("Show welcome notification on startup", &m_ShowWelcomeNotification))
    {
        // Setting changed
    }
    ImGui::TextDisabled("Display a notification when the framework initializes");

    ImGui::Spacing();

    if (ImGui::Checkbox("Auto-save settings on exit", &m_SaveSettingsOnExit))
    {
        // Setting changed
    }
    ImGui::TextDisabled("Automatically save all settings when the game closes");

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::SeparatorText("Actions");

    // Buttons for actions
    if (ImGui::Button("Save Settings Now"))
    {
        // Trigger save
        try
        {
            nlohmann::json config;
            SaveToConfig(config);
            Services::UIContext::Get().GetTheme().SaveToConfig(config);

            std::ofstream configFile("Broadsword.json");
            if (configFile.is_open())
            {
                configFile << config.dump(2);
                configFile.close();
                LOG_INFO("Settings saved to Broadsword.json");
            }
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("Failed to save settings: {}", e.what());
        }
    }
    ImGui::SameLine();

    if (ImGui::Button("Reset to Defaults"))
    {
        m_MenuToggleKey = VK_INSERT;
        m_ShowWelcomeNotification = true;
        m_SaveSettingsOnExit = true;
        LOG_INFO("General settings reset to defaults");
    }

    ImGui::EndChild();
}

void SettingsWindow::RenderThemeSettings()
{
    ImGui::BeginChild("ThemeSettings", ImVec2(0, 0), false);

    auto& theme = Services::UIContext::Get().GetTheme();
    auto& colors = const_cast<Services::ThemeColors&>(theme.GetColors());

    ImGui::SeparatorText("Framework Colors");

    if (RenderColorPicker("Accent Color", colors.accent, "Primary accent color for highlights"))
        m_ThemeModified = true;
    if (RenderColorPicker("Cursor Color", colors.cursor, "Text cursor color"))
        m_ThemeModified = true;
    if (RenderColorPicker("Background", colors.background, "Main background color"))
        m_ThemeModified = true;
    if (RenderColorPicker("Foreground", colors.foreground, "Main text color"))
        m_ThemeModified = true;

    ImGui::Spacing();
    ImGui::SeparatorText("Window & Interface");

    if (RenderColorPicker("Text", colors.text, "Primary text color"))
        m_ThemeModified = true;
    if (RenderColorPicker("Text Disabled", colors.text_disabled, "Disabled text color"))
        m_ThemeModified = true;
    if (RenderColorPicker("Window Background", colors.window_bg, "Window background"))
        m_ThemeModified = true;
    if (RenderColorPicker("Child Background", colors.child_bg, "Child window background"))
        m_ThemeModified = true;
    if (RenderColorPicker("Popup Background", colors.popup_bg, "Popup window background"))
        m_ThemeModified = true;
    if (RenderColorPicker("Border", colors.border, "Border color"))
        m_ThemeModified = true;

    ImGui::Spacing();
    ImGui::SeparatorText("Controls & Widgets");

    if (RenderColorPicker("Frame Background", colors.frame_bg, "Input field background"))
        m_ThemeModified = true;
    if (RenderColorPicker("Frame Hovered", colors.frame_bg_hovered, "Input field hovered"))
        m_ThemeModified = true;
    if (RenderColorPicker("Frame Active", colors.frame_bg_active, "Input field active"))
        m_ThemeModified = true;
    if (RenderColorPicker("Button", colors.button, "Button color"))
        m_ThemeModified = true;
    if (RenderColorPicker("Button Hovered", colors.button_hovered, "Button hover"))
        m_ThemeModified = true;
    if (RenderColorPicker("Button Active", colors.button_active, "Button pressed"))
        m_ThemeModified = true;
    if (RenderColorPicker("Check Mark", colors.check_mark, "Checkbox/radio checkmark"))
        m_ThemeModified = true;
    if (RenderColorPicker("Slider Grab", colors.slider_grab, "Slider handle"))
        m_ThemeModified = true;
    if (RenderColorPicker("Slider Grab Active", colors.slider_grab_active, "Slider handle active"))
        m_ThemeModified = true;

    ImGui::Spacing();
    ImGui::SeparatorText("Headers & Tabs");

    if (RenderColorPicker("Title Background", colors.title_bg, "Window title"))
        m_ThemeModified = true;
    if (RenderColorPicker("Title Active", colors.title_bg_active, "Active window title"))
        m_ThemeModified = true;
    if (RenderColorPicker("Header", colors.header, "Column header/tree node"))
        m_ThemeModified = true;
    if (RenderColorPicker("Header Hovered", colors.header_hovered, "Header hover"))
        m_ThemeModified = true;
    if (RenderColorPicker("Header Active", colors.header_active, "Header active"))
        m_ThemeModified = true;
    if (RenderColorPicker("Tab", colors.tab, "Tab button"))
        m_ThemeModified = true;
    if (RenderColorPicker("Tab Hovered", colors.tab_hovered, "Tab hover"))
        m_ThemeModified = true;
    if (RenderColorPicker("Tab Active", colors.tab_active, "Active tab"))
        m_ThemeModified = true;

    ImGui::Spacing();
    ImGui::SeparatorText("Terminal Colors (Normal)");

    if (RenderColorPicker("Black", colors.normal.black))
        m_ThemeModified = true;
    if (RenderColorPicker("Red", colors.normal.red))
        m_ThemeModified = true;
    if (RenderColorPicker("Green", colors.normal.green))
        m_ThemeModified = true;
    if (RenderColorPicker("Yellow", colors.normal.yellow))
        m_ThemeModified = true;
    if (RenderColorPicker("Blue", colors.normal.blue))
        m_ThemeModified = true;
    if (RenderColorPicker("Magenta", colors.normal.magenta))
        m_ThemeModified = true;
    if (RenderColorPicker("Cyan", colors.normal.cyan))
        m_ThemeModified = true;
    if (RenderColorPicker("White", colors.normal.white))
        m_ThemeModified = true;

    ImGui::Spacing();
    ImGui::SeparatorText("Terminal Colors (Bright)");

    if (RenderColorPicker("Bright Black", colors.bright.black))
        m_ThemeModified = true;
    if (RenderColorPicker("Bright Red", colors.bright.red))
        m_ThemeModified = true;
    if (RenderColorPicker("Bright Green", colors.bright.green))
        m_ThemeModified = true;
    if (RenderColorPicker("Bright Yellow", colors.bright.yellow))
        m_ThemeModified = true;
    if (RenderColorPicker("Bright Blue", colors.bright.blue))
        m_ThemeModified = true;
    if (RenderColorPicker("Bright Magenta", colors.bright.magenta))
        m_ThemeModified = true;
    if (RenderColorPicker("Bright Cyan", colors.bright.cyan))
        m_ThemeModified = true;
    if (RenderColorPicker("Bright White", colors.bright.white))
        m_ThemeModified = true;

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::SeparatorText("Actions");

    if (ImGui::Button("Apply Theme"))
    {
        theme.ApplyToImGui();
        m_ThemeModified = false;
        LOG_INFO("Theme applied to ImGui");
    }

    ImGui::SameLine();

    if (ImGui::Button("Save Theme"))
    {
        try
        {
            nlohmann::json config;
            theme.SaveToConfig(config);
            SaveToConfig(config);

            std::ofstream configFile("Broadsword.json");
            if (configFile.is_open())
            {
                configFile << config.dump(2);
                configFile.close();
                LOG_INFO("Theme saved to Broadsword.json");
            }
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("Failed to save theme: {}", e.what());
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Reset to Ayu Dark"))
    {
        colors = Services::Theme::GetAyuDarkDefaults();
        theme.ApplyToImGui();
        m_ThemeModified = false;
        LOG_INFO("Theme reset to Ayu Dark defaults");
    }

    if (m_ThemeModified)
    {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Theme modified - click 'Apply Theme' to see changes");
    }

    ImGui::EndChild();
}

void SettingsWindow::RenderLoggingSettings()
{
    ImGui::BeginChild("LoggingSettings", ImVec2(0, 0), false);

    ImGui::SeparatorText("Log Output");

    if (ImGui::Checkbox("Log to Console (stdout)", &m_LogToConsole))
    {
        Services::Logger::Get().SetOutputs(m_LogToConsole, m_LogToFile, m_LogToInGame);
    }
    ImGui::TextDisabled("Write logs to standard output console");

    ImGui::Spacing();

    if (ImGui::Checkbox("Log to File", &m_LogToFile))
    {
        Services::Logger::Get().SetOutputs(m_LogToConsole, m_LogToFile, m_LogToInGame);
    }
    ImGui::TextDisabled("Write logs to JSON files in Logs/ directory");

    ImGui::Spacing();

    if (ImGui::Checkbox("Log to In-Game Console", &m_LogToInGame))
    {
        Services::Logger::Get().SetOutputs(m_LogToConsole, m_LogToFile, m_LogToInGame);
    }
    ImGui::TextDisabled("Display logs in the in-game console window");

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::SeparatorText("Log Level");

    const char* logLevels[] = {"Trace", "Debug", "Info", "Warning", "Error", "Critical"};
    if (ImGui::Combo("Minimum Log Level", &m_MinLogLevel, logLevels, IM_ARRAYSIZE(logLevels)))
    {
        Services::Logger::Get().SetMinLevel(static_cast<Services::LogLevel>(m_MinLogLevel));
        LOG_INFO("Minimum log level set to {}", logLevels[m_MinLogLevel]);
    }
    ImGui::TextDisabled("Only logs at this level or higher will be recorded");

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::SeparatorText("File Rotation");

    if (ImGui::SliderInt("Max Log Files", &m_MaxLogFiles, 1, 20))
    {
        Services::Logger::Get().SetMaxFiles(m_MaxLogFiles);
    }
    ImGui::TextDisabled("Maximum number of log files to keep before deleting oldest");

    ImGui::Spacing();

    if (ImGui::SliderFloat("Max File Size (MB)", &m_MaxLogFileSizeMB, 1.0f, 500.0f, "%.1f MB"))
    {
        Services::Logger::Get().SetMaxFileSize(static_cast<size_t>(m_MaxLogFileSizeMB * 1024 * 1024));
    }
    ImGui::TextDisabled("Maximum size per log file before rotation");

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::SeparatorText("Information");

    ImGui::Text("Current Frame: %llu", Services::Logger::Get().GetCurrentFrame());

    char workingDir[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, workingDir);
    ImGui::Text("Log Directory: %s\\Logs", workingDir);

    ImGui::EndChild();
}


bool SettingsWindow::RenderColorPicker(const char* label, ImVec4& color, const char* description)
{
    bool changed = false;

    ImGui::PushID(label);

    // Color picker with alpha
    if (ImGui::ColorEdit4("##colorpicker", (float*)&color,
                          ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaBar))
    {
        changed = true;
    }

    ImGui::SameLine();
    ImGui::Text("%s", label);

    if (description)
    {
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("%s", description);
        }
    }

    ImGui::PopID();

    return changed;
}

bool SettingsWindow::RenderKeybindSelector(const char* label, int& keyCode, const char* description)
{
    bool changed = false;

    ImGui::PushID(label);

    // Button to start key capture
    const char* keyName = GetKeyName(keyCode);
    char buttonLabel[128];
    snprintf(buttonLabel, sizeof(buttonLabel), "%s##keybind", keyName);

    if (m_CapturingKey && m_CaptureTarget == keyCode)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.5f, 0.0f, 1.0f));
        if (ImGui::Button("Press any key...", ImVec2(150, 0)))
        {
            m_CapturingKey = false;
        }
        ImGui::PopStyleColor();

        // Capture key press
        for (int vk = 0; vk < 256; vk++)
        {
            if (vk == VK_ESCAPE)
                continue; // Skip ESC to allow canceling

            if (ImGui::IsKeyDown((ImGuiKey)vk))
            {
                keyCode = vk;
                m_CapturingKey = false;
                changed = true;
                LOG_INFO("Menu toggle key set to {}", GetKeyName(vk));
                break;
            }
        }

        // Cancel on ESC
        if (ImGui::IsKeyDown(ImGuiKey_Escape))
        {
            m_CapturingKey = false;
        }
    }
    else
    {
        if (ImGui::Button(buttonLabel, ImVec2(150, 0)))
        {
            m_CapturingKey = true;
            m_CaptureTarget = keyCode;
        }
    }

    ImGui::SameLine();
    ImGui::Text("%s", label);

    if (description)
    {
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("%s", description);
        }
    }

    ImGui::PopID();

    return changed;
}

const char* SettingsWindow::GetKeyName(int vkCode)
{
    static char buffer[32];

    switch (vkCode)
    {
    case VK_INSERT:
        return "INSERT";
    case VK_DELETE:
        return "DELETE";
    case VK_HOME:
        return "HOME";
    case VK_END:
        return "END";
    case VK_PRIOR:
        return "PAGE UP";
    case VK_NEXT:
        return "PAGE DOWN";
    case VK_F1:
        return "F1";
    case VK_F2:
        return "F2";
    case VK_F3:
        return "F3";
    case VK_F4:
        return "F4";
    case VK_F5:
        return "F5";
    case VK_F6:
        return "F6";
    case VK_F7:
        return "F7";
    case VK_F8:
        return "F8";
    case VK_F9:
        return "F9";
    case VK_F10:
        return "F10";
    case VK_F11:
        return "F11";
    case VK_F12:
        return "F12";
    case VK_ESCAPE:
        return "ESC";
    case VK_TAB:
        return "TAB";
    case VK_RETURN:
        return "ENTER";
    case VK_SPACE:
        return "SPACE";
    case VK_BACK:
        return "BACKSPACE";
    case VK_SHIFT:
        return "SHIFT";
    case VK_CONTROL:
        return "CTRL";
    case VK_MENU:
        return "ALT";
    default:
        if (vkCode >= '0' && vkCode <= '9')
        {
            snprintf(buffer, sizeof(buffer), "%c", (char)vkCode);
            return buffer;
        }
        if (vkCode >= 'A' && vkCode <= 'Z')
        {
            snprintf(buffer, sizeof(buffer), "%c", (char)vkCode);
            return buffer;
        }
        snprintf(buffer, sizeof(buffer), "VK_%d", vkCode);
        return buffer;
    }
}

void SettingsWindow::LoadFromConfig(const nlohmann::json& config)
{
    if (!config.contains("settings"))
    {
        return;
    }

    const auto& settings = config["settings"];

    // General settings
    if (settings.contains("menu_toggle_key"))
    {
        m_MenuToggleKey = settings["menu_toggle_key"].get<int>();
    }

    if (settings.contains("show_welcome_notification"))
    {
        m_ShowWelcomeNotification = settings["show_welcome_notification"].get<bool>();
    }

    if (settings.contains("save_settings_on_exit"))
    {
        m_SaveSettingsOnExit = settings["save_settings_on_exit"].get<bool>();
    }

    // Logging settings
    if (settings.contains("min_log_level"))
    {
        m_MinLogLevel = settings["min_log_level"].get<int>();
        Services::Logger::Get().SetMinLevel(static_cast<Services::LogLevel>(m_MinLogLevel));
    }

    if (settings.contains("log_to_console"))
    {
        m_LogToConsole = settings["log_to_console"].get<bool>();
    }

    if (settings.contains("log_to_file"))
    {
        m_LogToFile = settings["log_to_file"].get<bool>();
    }

    if (settings.contains("log_to_in_game"))
    {
        m_LogToInGame = settings["log_to_in_game"].get<bool>();
    }

    Services::Logger::Get().SetOutputs(m_LogToConsole, m_LogToFile, m_LogToInGame);

    if (settings.contains("max_log_files"))
    {
        m_MaxLogFiles = settings["max_log_files"].get<int>();
        Services::Logger::Get().SetMaxFiles(m_MaxLogFiles);
    }

    if (settings.contains("max_log_file_size_mb"))
    {
        m_MaxLogFileSizeMB = settings["max_log_file_size_mb"].get<float>();
        Services::Logger::Get().SetMaxFileSize(static_cast<size_t>(m_MaxLogFileSizeMB * 1024 * 1024));
    }
}

void SettingsWindow::SaveToConfig(nlohmann::json& config) const
{
    auto& settings = config["settings"];

    // General settings
    settings["menu_toggle_key"] = m_MenuToggleKey;
    settings["show_welcome_notification"] = m_ShowWelcomeNotification;
    settings["save_settings_on_exit"] = m_SaveSettingsOnExit;

    // Logging settings
    settings["min_log_level"] = m_MinLogLevel;
    settings["log_to_console"] = m_LogToConsole;
    settings["log_to_file"] = m_LogToFile;
    settings["log_to_in_game"] = m_LogToInGame;
    settings["max_log_files"] = m_MaxLogFiles;
    settings["max_log_file_size_mb"] = m_MaxLogFileSizeMB;
}

} // namespace Broadsword::Framework
