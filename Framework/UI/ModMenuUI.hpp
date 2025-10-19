#pragma once

#include <imgui.h>
#include <string>

namespace Broadsword::Framework {

class ConsoleWindow;

class ModMenuUI {
public:
    ModMenuUI();

    void Render();

    void SetConsoleWindow(ConsoleWindow* console) { m_ConsoleWindow = console; }

    void SetVisible(bool visible) { m_Visible = visible; }
    bool IsVisible() const { return m_Visible; }

    void ToggleVisible() { m_Visible = !m_Visible; }

private:
    bool m_Visible = true;
    char m_SearchBuffer[256] = {0};
    bool m_ShowFrameworkWindows = true;
    ConsoleWindow* m_ConsoleWindow = nullptr;
};

} // namespace Broadsword::Framework
