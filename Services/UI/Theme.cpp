#include "Theme.hpp"
#include <algorithm>
#include <cctype>
#include <cstdlib>

namespace Broadsword::Services {

Theme::Theme()
{
    m_Colors = GetAyuDarkDefaults();
}

void Theme::LoadFromConfig(const nlohmann::json& config)
{
    if (!config.contains("theme"))
    {
        m_Colors = GetAyuDarkDefaults();
        return;
    }

    const auto& theme = config["theme"];
    ThemeColors defaults = GetAyuDarkDefaults();

    // Helper lambda to load ImVec4 from JSON or use default
    auto loadColor = [&theme, &defaults](const std::string& key, const ImVec4& defaultVal) -> ImVec4 {
        if (!theme.contains(key))
            return defaultVal;

        const auto& colorJson = theme[key];
        if (colorJson.is_string())
        {
            // Hex string format
            return HexToColor(colorJson.get<std::string>());
        }
        else if (colorJson.is_array() && colorJson.size() >= 3)
        {
            // [r, g, b, a] array format
            float r = colorJson[0].get<float>();
            float g = colorJson[1].get<float>();
            float b = colorJson[2].get<float>();
            float a = colorJson.size() > 3 ? colorJson[3].get<float>() : 1.0f;
            return ImVec4(r, g, b, a);
        }

        return defaultVal;
    };

    // Load framework colors
    m_Colors.accent = loadColor("accent", defaults.accent);
    m_Colors.cursor = loadColor("cursor", defaults.cursor);
    m_Colors.background = loadColor("background", defaults.background);
    m_Colors.foreground = loadColor("foreground", defaults.foreground);

    // Load ImGui colors
    m_Colors.text = loadColor("text", defaults.text);
    m_Colors.text_disabled = loadColor("text_disabled", defaults.text_disabled);
    m_Colors.window_bg = loadColor("window_bg", defaults.window_bg);
    m_Colors.child_bg = loadColor("child_bg", defaults.child_bg);
    m_Colors.popup_bg = loadColor("popup_bg", defaults.popup_bg);
    m_Colors.border = loadColor("border", defaults.border);
    m_Colors.border_shadow = loadColor("border_shadow", defaults.border_shadow);
    m_Colors.frame_bg = loadColor("frame_bg", defaults.frame_bg);
    m_Colors.frame_bg_hovered = loadColor("frame_bg_hovered", defaults.frame_bg_hovered);
    m_Colors.frame_bg_active = loadColor("frame_bg_active", defaults.frame_bg_active);
    m_Colors.title_bg = loadColor("title_bg", defaults.title_bg);
    m_Colors.title_bg_active = loadColor("title_bg_active", defaults.title_bg_active);
    m_Colors.title_bg_collapsed = loadColor("title_bg_collapsed", defaults.title_bg_collapsed);
    m_Colors.menu_bar_bg = loadColor("menu_bar_bg", defaults.menu_bar_bg);
    m_Colors.scrollbar_bg = loadColor("scrollbar_bg", defaults.scrollbar_bg);
    m_Colors.scrollbar_grab = loadColor("scrollbar_grab", defaults.scrollbar_grab);
    m_Colors.scrollbar_grab_hovered = loadColor("scrollbar_grab_hovered", defaults.scrollbar_grab_hovered);
    m_Colors.scrollbar_grab_active = loadColor("scrollbar_grab_active", defaults.scrollbar_grab_active);
    m_Colors.check_mark = loadColor("check_mark", defaults.check_mark);
    m_Colors.slider_grab = loadColor("slider_grab", defaults.slider_grab);
    m_Colors.slider_grab_active = loadColor("slider_grab_active", defaults.slider_grab_active);
    m_Colors.button = loadColor("button", defaults.button);
    m_Colors.button_hovered = loadColor("button_hovered", defaults.button_hovered);
    m_Colors.button_active = loadColor("button_active", defaults.button_active);
    m_Colors.header = loadColor("header", defaults.header);
    m_Colors.header_hovered = loadColor("header_hovered", defaults.header_hovered);
    m_Colors.header_active = loadColor("header_active", defaults.header_active);
    m_Colors.separator = loadColor("separator", defaults.separator);
    m_Colors.separator_hovered = loadColor("separator_hovered", defaults.separator_hovered);
    m_Colors.separator_active = loadColor("separator_active", defaults.separator_active);
    m_Colors.resize_grip = loadColor("resize_grip", defaults.resize_grip);
    m_Colors.resize_grip_hovered = loadColor("resize_grip_hovered", defaults.resize_grip_hovered);
    m_Colors.resize_grip_active = loadColor("resize_grip_active", defaults.resize_grip_active);
    m_Colors.tab = loadColor("tab", defaults.tab);
    m_Colors.tab_hovered = loadColor("tab_hovered", defaults.tab_hovered);
    m_Colors.tab_active = loadColor("tab_active", defaults.tab_active);
    m_Colors.tab_unfocused = loadColor("tab_unfocused", defaults.tab_unfocused);
    m_Colors.tab_unfocused_active = loadColor("tab_unfocused_active", defaults.tab_unfocused_active);
    m_Colors.plot_lines = loadColor("plot_lines", defaults.plot_lines);
    m_Colors.plot_lines_hovered = loadColor("plot_lines_hovered", defaults.plot_lines_hovered);
    m_Colors.plot_histogram = loadColor("plot_histogram", defaults.plot_histogram);
    m_Colors.plot_histogram_hovered = loadColor("plot_histogram_hovered", defaults.plot_histogram_hovered);
    m_Colors.table_header_bg = loadColor("table_header_bg", defaults.table_header_bg);
    m_Colors.table_border_strong = loadColor("table_border_strong", defaults.table_border_strong);
    m_Colors.table_border_light = loadColor("table_border_light", defaults.table_border_light);
    m_Colors.table_row_bg = loadColor("table_row_bg", defaults.table_row_bg);
    m_Colors.table_row_bg_alt = loadColor("table_row_bg_alt", defaults.table_row_bg_alt);
    m_Colors.text_selected_bg = loadColor("text_selected_bg", defaults.text_selected_bg);

    // Load terminal colors - normal
    m_Colors.normal.black = loadColor("terminal_normal_black", defaults.normal.black);
    m_Colors.normal.red = loadColor("terminal_normal_red", defaults.normal.red);
    m_Colors.normal.green = loadColor("terminal_normal_green", defaults.normal.green);
    m_Colors.normal.yellow = loadColor("terminal_normal_yellow", defaults.normal.yellow);
    m_Colors.normal.blue = loadColor("terminal_normal_blue", defaults.normal.blue);
    m_Colors.normal.magenta = loadColor("terminal_normal_magenta", defaults.normal.magenta);
    m_Colors.normal.cyan = loadColor("terminal_normal_cyan", defaults.normal.cyan);
    m_Colors.normal.white = loadColor("terminal_normal_white", defaults.normal.white);

    // Load terminal colors - bright
    m_Colors.bright.black = loadColor("terminal_bright_black", defaults.bright.black);
    m_Colors.bright.red = loadColor("terminal_bright_red", defaults.bright.red);
    m_Colors.bright.green = loadColor("terminal_bright_green", defaults.bright.green);
    m_Colors.bright.yellow = loadColor("terminal_bright_yellow", defaults.bright.yellow);
    m_Colors.bright.blue = loadColor("terminal_bright_blue", defaults.bright.blue);
    m_Colors.bright.magenta = loadColor("terminal_bright_magenta", defaults.bright.magenta);
    m_Colors.bright.cyan = loadColor("terminal_bright_cyan", defaults.bright.cyan);
    m_Colors.bright.white = loadColor("terminal_bright_white", defaults.bright.white);
}

void Theme::SaveToConfig(nlohmann::json& config) const
{
    auto& theme = config["theme"];

    // Helper lambda to save ImVec4 as hex string
    auto saveColor = [&theme](const std::string& key, const ImVec4& color) {
        char hex[8];
        snprintf(hex, sizeof(hex), "#%02x%02x%02x",
                 static_cast<int>(color.x * 255.0f),
                 static_cast<int>(color.y * 255.0f),
                 static_cast<int>(color.z * 255.0f));
        theme[key] = hex;
    };

    // Save framework colors
    saveColor("accent", m_Colors.accent);
    saveColor("cursor", m_Colors.cursor);
    saveColor("background", m_Colors.background);
    saveColor("foreground", m_Colors.foreground);

    // Save ImGui colors
    saveColor("text", m_Colors.text);
    saveColor("text_disabled", m_Colors.text_disabled);
    saveColor("window_bg", m_Colors.window_bg);
    saveColor("child_bg", m_Colors.child_bg);
    saveColor("popup_bg", m_Colors.popup_bg);
    saveColor("border", m_Colors.border);
    saveColor("border_shadow", m_Colors.border_shadow);
    saveColor("frame_bg", m_Colors.frame_bg);
    saveColor("frame_bg_hovered", m_Colors.frame_bg_hovered);
    saveColor("frame_bg_active", m_Colors.frame_bg_active);
    saveColor("title_bg", m_Colors.title_bg);
    saveColor("title_bg_active", m_Colors.title_bg_active);
    saveColor("title_bg_collapsed", m_Colors.title_bg_collapsed);
    saveColor("menu_bar_bg", m_Colors.menu_bar_bg);
    saveColor("scrollbar_bg", m_Colors.scrollbar_bg);
    saveColor("scrollbar_grab", m_Colors.scrollbar_grab);
    saveColor("scrollbar_grab_hovered", m_Colors.scrollbar_grab_hovered);
    saveColor("scrollbar_grab_active", m_Colors.scrollbar_grab_active);
    saveColor("check_mark", m_Colors.check_mark);
    saveColor("slider_grab", m_Colors.slider_grab);
    saveColor("slider_grab_active", m_Colors.slider_grab_active);
    saveColor("button", m_Colors.button);
    saveColor("button_hovered", m_Colors.button_hovered);
    saveColor("button_active", m_Colors.button_active);
    saveColor("header", m_Colors.header);
    saveColor("header_hovered", m_Colors.header_hovered);
    saveColor("header_active", m_Colors.header_active);
    saveColor("separator", m_Colors.separator);
    saveColor("separator_hovered", m_Colors.separator_hovered);
    saveColor("separator_active", m_Colors.separator_active);
    saveColor("resize_grip", m_Colors.resize_grip);
    saveColor("resize_grip_hovered", m_Colors.resize_grip_hovered);
    saveColor("resize_grip_active", m_Colors.resize_grip_active);
    saveColor("tab", m_Colors.tab);
    saveColor("tab_hovered", m_Colors.tab_hovered);
    saveColor("tab_active", m_Colors.tab_active);
    saveColor("tab_unfocused", m_Colors.tab_unfocused);
    saveColor("tab_unfocused_active", m_Colors.tab_unfocused_active);
    saveColor("plot_lines", m_Colors.plot_lines);
    saveColor("plot_lines_hovered", m_Colors.plot_lines_hovered);
    saveColor("plot_histogram", m_Colors.plot_histogram);
    saveColor("plot_histogram_hovered", m_Colors.plot_histogram_hovered);
    saveColor("table_header_bg", m_Colors.table_header_bg);
    saveColor("table_border_strong", m_Colors.table_border_strong);
    saveColor("table_border_light", m_Colors.table_border_light);
    saveColor("table_row_bg", m_Colors.table_row_bg);
    saveColor("table_row_bg_alt", m_Colors.table_row_bg_alt);
    saveColor("text_selected_bg", m_Colors.text_selected_bg);

    // Save terminal colors - normal
    saveColor("terminal_normal_black", m_Colors.normal.black);
    saveColor("terminal_normal_red", m_Colors.normal.red);
    saveColor("terminal_normal_green", m_Colors.normal.green);
    saveColor("terminal_normal_yellow", m_Colors.normal.yellow);
    saveColor("terminal_normal_blue", m_Colors.normal.blue);
    saveColor("terminal_normal_magenta", m_Colors.normal.magenta);
    saveColor("terminal_normal_cyan", m_Colors.normal.cyan);
    saveColor("terminal_normal_white", m_Colors.normal.white);

    // Save terminal colors - bright
    saveColor("terminal_bright_black", m_Colors.bright.black);
    saveColor("terminal_bright_red", m_Colors.bright.red);
    saveColor("terminal_bright_green", m_Colors.bright.green);
    saveColor("terminal_bright_yellow", m_Colors.bright.yellow);
    saveColor("terminal_bright_blue", m_Colors.bright.blue);
    saveColor("terminal_bright_magenta", m_Colors.bright.magenta);
    saveColor("terminal_bright_cyan", m_Colors.bright.cyan);
    saveColor("terminal_bright_white", m_Colors.bright.white);
}

void Theme::ApplyToImGui()
{
    ImGuiStyle& style = ImGui::GetStyle();

    // Apply all ImGui colors
    style.Colors[ImGuiCol_Text] = m_Colors.text;
    style.Colors[ImGuiCol_TextDisabled] = m_Colors.text_disabled;
    style.Colors[ImGuiCol_WindowBg] = m_Colors.window_bg;
    style.Colors[ImGuiCol_ChildBg] = m_Colors.child_bg;
    style.Colors[ImGuiCol_PopupBg] = m_Colors.popup_bg;
    style.Colors[ImGuiCol_Border] = m_Colors.border;
    style.Colors[ImGuiCol_BorderShadow] = m_Colors.border_shadow;
    style.Colors[ImGuiCol_FrameBg] = m_Colors.frame_bg;
    style.Colors[ImGuiCol_FrameBgHovered] = m_Colors.frame_bg_hovered;
    style.Colors[ImGuiCol_FrameBgActive] = m_Colors.frame_bg_active;
    style.Colors[ImGuiCol_TitleBg] = m_Colors.title_bg;
    style.Colors[ImGuiCol_TitleBgActive] = m_Colors.title_bg_active;
    style.Colors[ImGuiCol_TitleBgCollapsed] = m_Colors.title_bg_collapsed;
    style.Colors[ImGuiCol_MenuBarBg] = m_Colors.menu_bar_bg;
    style.Colors[ImGuiCol_ScrollbarBg] = m_Colors.scrollbar_bg;
    style.Colors[ImGuiCol_ScrollbarGrab] = m_Colors.scrollbar_grab;
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = m_Colors.scrollbar_grab_hovered;
    style.Colors[ImGuiCol_ScrollbarGrabActive] = m_Colors.scrollbar_grab_active;
    style.Colors[ImGuiCol_CheckMark] = m_Colors.check_mark;
    style.Colors[ImGuiCol_SliderGrab] = m_Colors.slider_grab;
    style.Colors[ImGuiCol_SliderGrabActive] = m_Colors.slider_grab_active;
    style.Colors[ImGuiCol_Button] = m_Colors.button;
    style.Colors[ImGuiCol_ButtonHovered] = m_Colors.button_hovered;
    style.Colors[ImGuiCol_ButtonActive] = m_Colors.button_active;
    style.Colors[ImGuiCol_Header] = m_Colors.header;
    style.Colors[ImGuiCol_HeaderHovered] = m_Colors.header_hovered;
    style.Colors[ImGuiCol_HeaderActive] = m_Colors.header_active;
    style.Colors[ImGuiCol_Separator] = m_Colors.separator;
    style.Colors[ImGuiCol_SeparatorHovered] = m_Colors.separator_hovered;
    style.Colors[ImGuiCol_SeparatorActive] = m_Colors.separator_active;
    style.Colors[ImGuiCol_ResizeGrip] = m_Colors.resize_grip;
    style.Colors[ImGuiCol_ResizeGripHovered] = m_Colors.resize_grip_hovered;
    style.Colors[ImGuiCol_ResizeGripActive] = m_Colors.resize_grip_active;
    style.Colors[ImGuiCol_Tab] = m_Colors.tab;
    style.Colors[ImGuiCol_TabHovered] = m_Colors.tab_hovered;
    style.Colors[ImGuiCol_TabActive] = m_Colors.tab_active;
    style.Colors[ImGuiCol_TabUnfocused] = m_Colors.tab_unfocused;
    style.Colors[ImGuiCol_TabUnfocusedActive] = m_Colors.tab_unfocused_active;
    style.Colors[ImGuiCol_PlotLines] = m_Colors.plot_lines;
    style.Colors[ImGuiCol_PlotLinesHovered] = m_Colors.plot_lines_hovered;
    style.Colors[ImGuiCol_PlotHistogram] = m_Colors.plot_histogram;
    style.Colors[ImGuiCol_PlotHistogramHovered] = m_Colors.plot_histogram_hovered;
    style.Colors[ImGuiCol_TableHeaderBg] = m_Colors.table_header_bg;
    style.Colors[ImGuiCol_TableBorderStrong] = m_Colors.table_border_strong;
    style.Colors[ImGuiCol_TableBorderLight] = m_Colors.table_border_light;
    style.Colors[ImGuiCol_TableRowBg] = m_Colors.table_row_bg;
    style.Colors[ImGuiCol_TableRowBgAlt] = m_Colors.table_row_bg_alt;
    style.Colors[ImGuiCol_TextSelectedBg] = m_Colors.text_selected_bg;
}

ImVec4 Theme::HexToColor(std::string_view hex)
{
    std::string hexStr(hex);

    // Remove '#' prefix if present
    if (!hexStr.empty() && hexStr[0] == '#')
    {
        hexStr = hexStr.substr(1);
    }

    // Convert to lowercase for parsing
    std::transform(hexStr.begin(), hexStr.end(), hexStr.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    // Parse RGB components
    if (hexStr.length() != 6)
    {
        return ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // White fallback
    }

    unsigned int r = std::strtoul(hexStr.substr(0, 2).c_str(), nullptr, 16);
    unsigned int g = std::strtoul(hexStr.substr(2, 2).c_str(), nullptr, 16);
    unsigned int b = std::strtoul(hexStr.substr(4, 2).c_str(), nullptr, 16);

    return ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
}

ThemeColors Theme::GetAyuDarkDefaults()
{
    ThemeColors colors;

    // Framework colors (from user's Ayu Dark YAML)
    colors.accent = HexToColor("#e6b450");
    colors.cursor = HexToColor("#e6b450");
    colors.background = HexToColor("#0b0e14");
    colors.foreground = HexToColor("#bfbdb6");

    // Terminal colors - normal
    colors.normal.black = HexToColor("#1e232b");
    colors.normal.red = HexToColor("#ea6c73");
    colors.normal.green = HexToColor("#7fd962");
    colors.normal.yellow = HexToColor("#f9af4f");
    colors.normal.blue = HexToColor("#53bdfa");
    colors.normal.magenta = HexToColor("#cda1fa");
    colors.normal.cyan = HexToColor("#90e1c6");
    colors.normal.white = HexToColor("#c7c7c7");

    // Terminal colors - bright
    colors.bright.black = HexToColor("#686868");
    colors.bright.red = HexToColor("#f07178");
    colors.bright.green = HexToColor("#aad94c");
    colors.bright.yellow = HexToColor("#ffb454");
    colors.bright.blue = HexToColor("#59c2ff");
    colors.bright.magenta = HexToColor("#d2a6ff");
    colors.bright.cyan = HexToColor("#95e6cb");
    colors.bright.white = HexToColor("#ffffff");

    // ImGui color mapping derived from Ayu Dark palette
    colors.text = colors.foreground;
    colors.text_disabled = HexToColor("#3e4b59");
    colors.window_bg = ImVec4(colors.background.x, colors.background.y, colors.background.z, 0.95f);
    colors.child_bg = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    colors.popup_bg = ImVec4(colors.background.x * 1.1f, colors.background.y * 1.1f, colors.background.z * 1.1f, 0.98f);
    colors.border = HexToColor("#1f2430");
    colors.border_shadow = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    colors.frame_bg = HexToColor("#0d1016");
    colors.frame_bg_hovered = HexToColor("#1a1f29");
    colors.frame_bg_active = HexToColor("#1f2430");
    colors.title_bg = HexToColor("#0b0e14");
    colors.title_bg_active = HexToColor("#0d1016");
    colors.title_bg_collapsed = HexToColor("#0b0e14");
    colors.menu_bar_bg = HexToColor("#0d1016");
    colors.scrollbar_bg = HexToColor("#0d1016");
    colors.scrollbar_grab = HexToColor("#1f2430");
    colors.scrollbar_grab_hovered = HexToColor("#2d3340");
    colors.scrollbar_grab_active = HexToColor("#3e4b59");
    colors.check_mark = colors.accent;
    colors.slider_grab = colors.accent;
    colors.slider_grab_active = ImVec4(colors.accent.x * 1.2f, colors.accent.y * 1.2f, colors.accent.z * 1.2f, 1.0f);
    colors.button = HexToColor("#1f2430");
    colors.button_hovered = HexToColor("#2d3340");
    colors.button_active = HexToColor("#3e4b59");
    colors.header = HexToColor("#1f2430");
    colors.header_hovered = HexToColor("#2d3340");
    colors.header_active = HexToColor("#3e4b59");
    colors.separator = HexToColor("#1f2430");
    colors.separator_hovered = colors.accent;
    colors.separator_active = ImVec4(colors.accent.x * 1.2f, colors.accent.y * 1.2f, colors.accent.z * 1.2f, 1.0f);
    colors.resize_grip = HexToColor("#1f2430");
    colors.resize_grip_hovered = colors.accent;
    colors.resize_grip_active = ImVec4(colors.accent.x * 1.2f, colors.accent.y * 1.2f, colors.accent.z * 1.2f, 1.0f);
    colors.tab = HexToColor("#0d1016");
    colors.tab_hovered = HexToColor("#2d3340");
    colors.tab_active = HexToColor("#1f2430");
    colors.tab_unfocused = HexToColor("#0b0e14");
    colors.tab_unfocused_active = HexToColor("#1a1f29");
    colors.plot_lines = colors.accent;
    colors.plot_lines_hovered = colors.normal.yellow;
    colors.plot_histogram = colors.normal.blue;
    colors.plot_histogram_hovered = colors.bright.blue;
    colors.table_header_bg = HexToColor("#1f2430");
    colors.table_border_strong = HexToColor("#1f2430");
    colors.table_border_light = HexToColor("#0d1016");
    colors.table_row_bg = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    colors.table_row_bg_alt = ImVec4(1.0f, 1.0f, 1.0f, 0.03f);
    colors.text_selected_bg = ImVec4(colors.accent.x, colors.accent.y, colors.accent.z, 0.35f);

    return colors;
}

} // namespace Broadsword::Services
