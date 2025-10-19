#pragma once

#include "Theme.hpp"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace Broadsword::Services {

struct ModUIElement {
    std::string modName;
    std::string displayName;
    std::function<void()> renderCallback;
    bool enabled;
};

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

private:
    UIContext() = default;

    Theme m_Theme;
    std::vector<ModUIElement> m_ModUIElements;
};

} // namespace Broadsword::Services
