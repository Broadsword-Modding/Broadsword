#pragma once

#include <imgui.h>

namespace Broadsword::Framework {

class AboutWindow {
public:
    AboutWindow();

    void Render();

    void SetVisible(bool visible) { m_Visible = visible; }
    bool IsVisible() const { return m_Visible; }

private:
    bool m_Visible = false;
};

} // namespace Broadsword::Framework
