#pragma once

#include "Theme.hpp"
#include "BindingManager.hpp"
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <source_location>

namespace Broadsword::Services {

struct ModUIElement {
    std::string modName;
    std::string displayName;
    std::function<void()> renderCallback;
    bool enabled;
};

/**
 * UIContext - Unified UI service with ImGui wrappers and keybinding
 *
 * Provides ImGui widget wrappers that automatically integrate with
 * BindingManager for universal keybinding support.
 *
 * Features:
 * - Automatic keybinding registration using std::source_location
 * - Right-click on widgets to bind keys
 * - Bound keys trigger widgets even when not focused
 * - Persistent bindings across sessions
 *
 * Usage:
 *   if (ctx.ui.Button("Toggle Feature")) {
 *       // Triggered by click OR bound key
 *   }
 */
class UIContext {
public:
    static UIContext& Get();

    UIContext(const UIContext&) = delete;
    UIContext& operator=(const UIContext&) = delete;

    void Initialize();
    void Shutdown();

    Theme& GetTheme() { return m_Theme; }
    const Theme& GetTheme() const { return m_Theme; }

    void RegisterModUI(const std::string& modName, const std::string& displayName,
                       std::function<void()> renderCallback);

    void UnregisterModUI(const std::string& modName);

    const std::vector<ModUIElement>& GetModUIElements() const { return m_ModUIElements; }

    void SetModUIEnabled(const std::string& modName, bool enabled);
    bool IsModUIEnabled(const std::string& modName) const;

    void RenderModUIs();

    /**
     * Get binding manager instance
     */
    BindingManager& GetBindingManager() { return m_BindingManager; }

    /**
     * Set current mod name for keybinding context
     *
     * Must be called before using Button, Checkbox, etc.
     * Usually called once per mod at the start of UI rendering.
     *
     * @param modName Name of mod rendering UI
     */
    void SetCurrentMod(std::string_view modName);

    /**
     * Button with automatic keybinding support
     *
     * Automatically registers with BindingManager using source location.
     * Right-click to show binding popup.
     * Returns true if clicked OR bound key pressed.
     *
     * @param label Button label
     * @param loc Source location (auto-captured)
     * @return true if activated (clicked or key pressed)
     */
    bool Button(
        std::string_view label,
        std::source_location loc = std::source_location::current()
    );

    /**
     * Update bindings (called once per frame before UI rendering)
     */
    void UpdateBindings();

private:
    UIContext() = default;

    /**
     * Show keybinding popup for a widget
     *
     * @param id Binding ID
     */
    void ShowBindingPopup(const BindingID& id);

    Theme m_Theme;
    std::vector<ModUIElement> m_ModUIElements;
    BindingManager m_BindingManager;
    std::string m_CurrentModName;
};

} // namespace Broadsword::Services
