#include "ModMenuUI.hpp"
#include "../../Services/UI/UIContext.hpp"
#include "ConsoleWindow.hpp"
#include <algorithm>

namespace Broadsword::Framework {

ModMenuUI::ModMenuUI()
{
}

void ModMenuUI::Render()
{
    if (!m_Visible)
    {
        return;
    }

    const auto& theme = Services::UIContext::Get().GetTheme().GetColors();

    // Set size constraints: width 300-500px, height unlimited
    // ImGui format: SetNextWindowSizeConstraints(min_size, max_size)
    ImGui::SetNextWindowSizeConstraints(
        ImVec2(300, 100),      // Min: 300px wide, 100px tall
        ImVec2(500, FLT_MAX)   // Max: 500px wide, unlimited height
    );
    ImGui::SetNextWindowSize(ImVec2(400, 700), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Broadsword Framework", &m_Visible))
    {
        ImGui::End();
        return;
    }

    // Framework options inline at the top
    const float windowWidth = ImGui::GetContentRegionAvail().x;
    const float buttonSpacing = 10.0f;
    const int numButtons = 3;
    const float totalSpacing = buttonSpacing * (numButtons - 1);
    const float buttonWidth = (windowWidth - totalSpacing) / numButtons;

    // Console button
    {
        bool isOpen = m_ConsoleWindow && m_ConsoleWindow->IsVisible();
        bool isHovered = false;

        ImVec2 textPos = ImGui::GetCursorScreenPos();
        const char* label = "Console";
        ImVec2 textSize = ImGui::CalcTextSize(label);

        // Center text within button area
        float textX = textPos.x + (buttonWidth - textSize.x) / 2.0f;

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

        if (ImGui::Button("##Console", ImVec2(buttonWidth, textSize.y)))
        {
            if (m_ConsoleWindow)
            {
                m_ConsoleWindow->SetVisible(!m_ConsoleWindow->IsVisible());
            }
        }
        isHovered = ImGui::IsItemHovered();

        ImGui::PopStyleColor(3);

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec4 textColor = (isOpen || isHovered) ? theme.accent : theme.text;

        if (isOpen || isHovered)
        {
            drawList->AddText(ImVec2(textX + 0.5f, textPos.y), ImGui::GetColorU32(textColor), label);
        }
        drawList->AddText(ImVec2(textX, textPos.y), ImGui::GetColorU32(textColor), label);

        drawList->AddLine(
            ImVec2(textX, textPos.y + textSize.y),
            ImVec2(textX + textSize.x, textPos.y + textSize.y),
            ImGui::GetColorU32(textColor),
            1.0f
        );

        ImGui::SameLine(0, buttonSpacing);
    }

    // Settings button
    {
        bool isHovered = false;

        ImVec2 textPos = ImGui::GetCursorScreenPos();
        const char* label = "Settings";
        ImVec2 textSize = ImGui::CalcTextSize(label);

        float textX = textPos.x + (buttonWidth - textSize.x) / 2.0f;

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

        if (ImGui::Button("##Settings", ImVec2(buttonWidth, textSize.y)))
        {
            // Open settings window (future implementation)
        }
        isHovered = ImGui::IsItemHovered();

        ImGui::PopStyleColor(3);

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec4 textColor = isHovered ? theme.accent : theme.text;

        if (isHovered)
        {
            drawList->AddText(ImVec2(textX + 0.5f, textPos.y), ImGui::GetColorU32(textColor), label);
        }
        drawList->AddText(ImVec2(textX, textPos.y), ImGui::GetColorU32(textColor), label);

        drawList->AddLine(
            ImVec2(textX, textPos.y + textSize.y),
            ImVec2(textX + textSize.x, textPos.y + textSize.y),
            ImGui::GetColorU32(textColor),
            1.0f
        );

        ImGui::SameLine(0, buttonSpacing);
    }

    // About button
    {
        bool isHovered = false;

        ImVec2 textPos = ImGui::GetCursorScreenPos();
        const char* label = "About";
        ImVec2 textSize = ImGui::CalcTextSize(label);

        float textX = textPos.x + (buttonWidth - textSize.x) / 2.0f;

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

        if (ImGui::Button("##About", ImVec2(buttonWidth, textSize.y)))
        {
            // Open about window (future implementation)
        }
        isHovered = ImGui::IsItemHovered();

        ImGui::PopStyleColor(3);

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec4 textColor = isHovered ? theme.accent : theme.text;

        if (isHovered)
        {
            drawList->AddText(ImVec2(textX + 0.5f, textPos.y), ImGui::GetColorU32(textColor), label);
        }
        drawList->AddText(ImVec2(textX, textPos.y), ImGui::GetColorU32(textColor), label);

        drawList->AddLine(
            ImVec2(textX, textPos.y + textSize.y),
            ImVec2(textX + textSize.x, textPos.y + textSize.y),
            ImGui::GetColorU32(textColor),
            1.0f
        );
    }

    ImGui::Spacing();
    ImGui::Separator();

    // Search bar
    ImGui::SetNextItemWidth(-1);
    ImGui::InputTextWithHint("##ModSearch", "Search mods...", m_SearchBuffer, sizeof(m_SearchBuffer));

    ImGui::Spacing();

    // Mods section (no dropdown)
    const auto& modUIElements = Services::UIContext::Get().GetModUIElements();
    const std::string searchStr = m_SearchBuffer;

    if (modUIElements.empty())
    {
        // Center the "No mods loaded :(" text vertically and horizontally
        const char* emptyText = "No mods loaded :(";
        ImVec2 textSize = ImGui::CalcTextSize(emptyText);
        ImVec2 availSize = ImGui::GetContentRegionAvail();

        // Calculate centered position
        float textX = (availSize.x - textSize.x) / 2.0f;
        float textY = (availSize.y - textSize.y) / 2.0f;

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + textX);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + textY);

        ImGui::TextDisabled("%s", emptyText);
    }
    else
    {
        for (const auto& element : modUIElements)
        {
            // Apply search filter
            if (!searchStr.empty())
            {
                std::string nameLower = element.displayName;
                std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);

                std::string searchLower = searchStr;
                std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(),
                               ::tolower);

                if (nameLower.find(searchLower) == std::string::npos)
                {
                    continue;
                }
            }

            // Mod entry with enable/disable checkbox
            bool enabled = element.enabled;
            if (ImGui::Checkbox(element.displayName.c_str(), &enabled))
            {
                Services::UIContext::Get().SetModUIEnabled(element.modName, enabled);
            }

            // Right-click context menu
            if (ImGui::BeginPopupContextItem())
            {
                ImGui::TextColored(theme.accent, "%s", element.displayName.c_str());
                ImGui::Separator();

                if (ImGui::MenuItem("Enable"))
                {
                    Services::UIContext::Get().SetModUIEnabled(element.modName, true);
                }

                if (ImGui::MenuItem("Disable"))
                {
                    Services::UIContext::Get().SetModUIEnabled(element.modName, false);
                }

                ImGui::Separator();

                ImGui::TextDisabled("Mod: %s", element.modName.c_str());

                ImGui::EndPopup();
            }
        }
    }

    ImGui::End();
}

} // namespace Broadsword::Framework
