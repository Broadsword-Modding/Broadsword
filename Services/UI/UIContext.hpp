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
     * Checkbox with automatic keybinding support
     *
     * Toggles value when clicked OR bound key pressed.
     * Right-click to bind key.
     *
     * @param label Checkbox label
     * @param value Pointer to bool value
     * @param loc Source location (auto-captured)
     * @return true if value changed
     */
    bool Checkbox(
        std::string_view label,
        bool* value,
        std::source_location loc = std::source_location::current()
    );

    /**
     * Slider with automatic keybinding support
     *
     * Bound key increments value by step amount.
     * Shift+bound key decrements value.
     * Right-click to bind key.
     *
     * @param label Slider label
     * @param value Pointer to float value
     * @param min Minimum value
     * @param max Maximum value
     * @param step Step amount for key increment (default: (max-min)/10)
     * @param loc Source location (auto-captured)
     * @return true if value changed
     */
    bool SliderFloat(
        std::string_view label,
        float* value,
        float min,
        float max,
        float step = 0.0f,
        std::source_location loc = std::source_location::current()
    );

    /**
     * Combo box with automatic keybinding support
     *
     * Bound key cycles to next item.
     * Shift+bound key cycles to previous item.
     * Right-click to bind key.
     *
     * @param label Combo label
     * @param currentItem Pointer to current item index
     * @param items Array of item labels
     * @param itemCount Number of items
     * @param loc Source location (auto-captured)
     * @return true if selection changed
     */
    bool Combo(
        std::string_view label,
        int* currentItem,
        const char* const items[],
        int itemCount,
        std::source_location loc = std::source_location::current()
    );

    /**
     * Integer slider with automatic keybinding support
     *
     * Bound key increments value by 1.
     * Shift+bound key decrements value by 1.
     * Right-click to bind key.
     *
     * @param label Slider label
     * @param value Pointer to int value
     * @param min Minimum value
     * @param max Maximum value
     * @param loc Source location (auto-captured)
     * @return true if value changed
     */
    bool SliderInt(
        std::string_view label,
        int* value,
        int min,
        int max,
        std::source_location loc = std::source_location::current()
    );

    /**
     * Begin a tab bar
     *
     * @param id Tab bar ID
     * @return true if tab bar is open
     */
    bool BeginTabBar(std::string_view id);

    /**
     * Begin a tab item
     *
     * @param label Tab label
     * @return true if tab is selected
     */
    bool BeginTabItem(std::string_view label);

    /**
     * End a tab item (call after BeginTabItem returns true)
     */
    void EndTabItem();

    /**
     * End a tab bar (call after BeginTabBar)
     */
    void EndTabBar();

    /**
     * Display text
     *
     * @param text Text to display
     */
    void Text(std::string_view text);

    /**
     * Display colored text
     *
     * @param color Text color (RGBA 0.0-1.0)
     * @param text Text to display
     */
    void TextColored(const ImVec4& color, std::string_view text);

    /**
     * Display wrapped text that breaks lines automatically
     *
     * @param text Text to display
     */
    void TextWrapped(std::string_view text);

    /**
     * Draw a horizontal separator line
     */
    void Separator();

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
