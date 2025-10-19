#include "UIContext.hpp"
#include <algorithm>

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

} // namespace Broadsword::Services
