#include "UIContext.hpp"
#include <algorithm>
#include <imgui.h>
#include <Windows.h>

namespace Broadsword::Services {

UIContext& UIContext::Get()
{
    static UIContext instance;
    return instance;
}

void UIContext::Initialize()
{
    // Theme will load defaults on construction
    // Future: Load theme from config here
}

void UIContext::Shutdown()
{
    m_ModUIElements.clear();
}

void UIContext::RegisterModUI(const std::string& modName, const std::string& displayName,
                               std::function<void()> renderCallback)
{
    // Check if mod already registered
    auto it = std::find_if(m_ModUIElements.begin(), m_ModUIElements.end(),
                           [&modName](const ModUIElement& elem) { return elem.modName == modName; });

    if (it != m_ModUIElements.end())
    {
        // Update existing registration
        it->displayName = displayName;
        it->renderCallback = std::move(renderCallback);
        return;
    }

    // Add new mod UI element
    m_ModUIElements.push_back(ModUIElement{
        .modName = modName,
        .displayName = displayName,
        .renderCallback = std::move(renderCallback),
        .enabled = true,
    });
}

void UIContext::UnregisterModUI(const std::string& modName)
{
    m_ModUIElements.erase(std::remove_if(m_ModUIElements.begin(), m_ModUIElements.end(),
                                         [&modName](const ModUIElement& elem) { return elem.modName == modName; }),
                          m_ModUIElements.end());
}

void UIContext::SetModUIEnabled(const std::string& modName, bool enabled)
{
    auto it = std::find_if(m_ModUIElements.begin(), m_ModUIElements.end(),
                           [&modName](const ModUIElement& elem) { return elem.modName == modName; });

    if (it != m_ModUIElements.end())
    {
        it->enabled = enabled;
    }
}

bool UIContext::IsModUIEnabled(const std::string& modName) const
{
    auto it = std::find_if(m_ModUIElements.begin(), m_ModUIElements.end(),
                           [&modName](const ModUIElement& elem) { return elem.modName == modName; });

    return it != m_ModUIElements.end() && it->enabled;
}

void UIContext::RenderModUIs()
{
    for (const auto& element : m_ModUIElements)
    {
        if (element.enabled && element.renderCallback)
        {
            element.renderCallback();
        }
    }
}

void UIContext::SetCurrentMod(std::string_view modName)
{
    m_CurrentModName = modName;
}

void UIContext::UpdateBindings()
{
    m_BindingManager.Update();
}

bool UIContext::Button(std::string_view label, std::source_location loc)
{
    using namespace Broadsword;

    // Create binding ID
    BindingID id = BindingID::FromLocation(m_CurrentModName, label, loc);

    // Register if not already registered
    if (!m_BindingManager.IsRegistered(id)) {
        m_BindingManager.Register(id, std::string(label));
    }

    // Check if bound key was pressed
    bool keyPressed = m_BindingManager.WasKeyPressed(id);

    // Render button
    bool clicked = ImGui::Button(std::string(label).c_str());

    // Show keybinding indicator if bound
    auto boundKey = m_BindingManager.GetKey(id);
    if (boundKey.has_value()) {
        ImGui::SameLine();
        ImGui::TextDisabled("[VK %d]", boundKey.value());
    }

    // Right-click to show binding popup
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        ImGui::OpenPopup("BindKeyPopup");
    }

    // Show binding popup if open
    if (ImGui::BeginPopup("BindKeyPopup")) {
        ShowBindingPopup(id);
        ImGui::EndPopup();
    }

    // Return true if clicked OR key pressed
    return clicked || keyPressed;
}

void UIContext::ShowBindingPopup(const BindingID& id)
{
    ImGui::Text("Bind key for: %s", id.label.c_str());
    ImGui::Separator();

    auto currentKey = m_BindingManager.GetKey(id);
    if (currentKey.has_value()) {
        ImGui::Text("Current: VK %d", currentKey.value());
        if (ImGui::Button("Unbind")) {
            m_BindingManager.Unbind(id);
            ImGui::CloseCurrentPopup();
        }
    } else {
        ImGui::Text("No key bound");
    }

    ImGui::Separator();
    ImGui::Text("Press any key to bind...");
    ImGui::Text("Press ESC to cancel");

    // Check for key press
    for (int vk = 0x08; vk <= 0xFE; vk++) {
        // Skip mouse buttons and reserved keys
        if (vk >= VK_LBUTTON && vk <= VK_XBUTTON2) continue;
        if (vk == 0x0A || vk == 0x0B) continue; // Reserved
        if (vk >= 0x0E && vk <= 0x0F) continue; // Undefined
        if (vk >= 0x16 && vk <= 0x19) continue; // IME keys
        if (vk == 0x1A) continue; // Undefined
        if (vk >= 0x3A && vk <= 0x40) continue; // Undefined
        if (vk == 0x5E) continue; // Reserved
        if (vk >= 0x88 && vk <= 0x8F) continue; // Unassigned
        if (vk >= 0x97 && vk <= 0x9F) continue; // Unassigned
        if (vk >= 0xB8 && vk <= 0xB9) continue; // Reserved
        if (vk >= 0xC1 && vk <= 0xD7) continue; // Reserved
        if (vk >= 0xD8 && vk <= 0xDA) continue; // Unassigned
        if (vk >= 0xE0 && vk <= 0xE1) continue; // OEM specific
        if (vk == 0xE3 || vk == 0xE4) continue; // OEM specific
        if (vk >= 0xE6 && vk <= 0xE6) continue; // OEM specific
        if (vk >= 0xE9 && vk <= 0xF5) continue; // OEM specific
        if (vk == 0xFC || vk == 0xFD) continue; // Reserved

        if (GetAsyncKeyState(vk) & 0x8000) {
            // ESC cancels
            if (vk == VK_ESCAPE) {
                ImGui::CloseCurrentPopup();
                return;
            }

            // Bind this key
            m_BindingManager.Bind(id, vk);
            ImGui::CloseCurrentPopup();
            return;
        }
    }
}

} // namespace Broadsword::Services
