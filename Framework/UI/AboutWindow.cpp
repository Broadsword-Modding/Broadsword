#include "AboutWindow.hpp"
#include "../../Engine/SDK/SDK/Basic.hpp"

namespace Broadsword::Framework {

AboutWindow::AboutWindow()
{
}

void AboutWindow::Render()
{
    if (!m_Visible)
    {
        return;
    }

    // Fixed size window
    ImGui::SetNextWindowSize(ImVec2(350, 175), ImGuiCond_Always);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

    if (!ImGui::Begin("About Broadsword", &m_Visible, windowFlags))
    {
        ImGui::End();
        return;
    }

    ImGui::SeparatorText("Broadsword Framework");

    ImGui::Text("Version: 2.1.0");
    ImGui::Text("Build: %s",
#ifdef _DEBUG
                "Debug"
#else
                "Release"
#endif
    );

    ImGui::Spacing();

    ImGui::Text("ImGui: %s", ImGui::GetVersion());
    ImGui::Text("Platform: Windows x64");

    ImGui::Spacing();

    ImGui::SeparatorText("SDK Information");

    ImGui::Text("GWorld Offset: 0x%08X", SDK::Offsets::GWorld);

    ImGui::End();
}

} // namespace Broadsword::Framework
