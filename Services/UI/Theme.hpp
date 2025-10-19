#pragma once

#include <imgui.h>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>

namespace Broadsword::Services {

struct TerminalColors {
    ImVec4 black;
    ImVec4 red;
    ImVec4 green;
    ImVec4 yellow;
    ImVec4 blue;
    ImVec4 magenta;
    ImVec4 cyan;
    ImVec4 white;
};

struct ThemeColors {
    // Framework colors
    ImVec4 accent;
    ImVec4 cursor;
    ImVec4 background;
    ImVec4 foreground;

    // ImGui color mapping (all ImGuiCol_ enum values)
    ImVec4 text;
    ImVec4 text_disabled;
    ImVec4 window_bg;
    ImVec4 child_bg;
    ImVec4 popup_bg;
    ImVec4 border;
    ImVec4 border_shadow;
    ImVec4 frame_bg;
    ImVec4 frame_bg_hovered;
    ImVec4 frame_bg_active;
    ImVec4 title_bg;
    ImVec4 title_bg_active;
    ImVec4 title_bg_collapsed;
    ImVec4 menu_bar_bg;
    ImVec4 scrollbar_bg;
    ImVec4 scrollbar_grab;
    ImVec4 scrollbar_grab_hovered;
    ImVec4 scrollbar_grab_active;
    ImVec4 check_mark;
    ImVec4 slider_grab;
    ImVec4 slider_grab_active;
    ImVec4 button;
    ImVec4 button_hovered;
    ImVec4 button_active;
    ImVec4 header;
    ImVec4 header_hovered;
    ImVec4 header_active;
    ImVec4 separator;
    ImVec4 separator_hovered;
    ImVec4 separator_active;
    ImVec4 resize_grip;
    ImVec4 resize_grip_hovered;
    ImVec4 resize_grip_active;
    ImVec4 tab;
    ImVec4 tab_hovered;
    ImVec4 tab_active;
    ImVec4 tab_unfocused;
    ImVec4 tab_unfocused_active;
    ImVec4 plot_lines;
    ImVec4 plot_lines_hovered;
    ImVec4 plot_histogram;
    ImVec4 plot_histogram_hovered;
    ImVec4 table_header_bg;
    ImVec4 table_border_strong;
    ImVec4 table_border_light;
    ImVec4 table_row_bg;
    ImVec4 table_row_bg_alt;
    ImVec4 text_selected_bg;

    // Terminal colors (for console log levels)
    TerminalColors normal;
    TerminalColors bright;

    // Helper: Get semantic colors
    ImVec4 Success() const { return normal.green; }
    ImVec4 Error() const { return normal.red; }
    ImVec4 Warning() const { return normal.yellow; }
    ImVec4 Info() const { return normal.blue; }
};

class Theme {
public:
    Theme();

    // Load theme colors from config JSON
    void LoadFromConfig(const nlohmann::json& config);

    // Save theme colors to config JSON
    void SaveToConfig(nlohmann::json& config) const;

    // Apply theme colors to ImGui style
    void ApplyToImGui();

    // Get theme colors
    const ThemeColors& GetColors() const { return m_Colors; }

    // Helper: Convert hex string to ImVec4 (e.g., "#e6b450" -> ImVec4)
    static ImVec4 HexToColor(std::string_view hex);

    // Get default Ayu Dark theme
    static ThemeColors GetAyuDarkDefaults();

private:
    ThemeColors m_Colors;
};

} // namespace Broadsword::Services
