#include "ContentBrowserPanel.h"

#include "core/Errors.h"
#include "core/editor/Editor.h"
#include "core/editor/LiveSettings.h"
#include "core/windows/MainWindow.h"

#include "panels/PanelManager.h"

#include <imgui.h>

namespace oly::editor
{
	ContentBrowserPanel& ContentBrowserPanel::Instance()
	{
		if (auto panel = MainWindow::Instance().GetPanelManager().Get<ContentBrowserPanel>())
			return *panel;
		else
			BreakoutError::Throw("No instance of ContentBrowserPanel");
	}

	void ContentBrowserPanel::InitImpl()
	{
		// NOP
	}

	const char* ContentBrowserPanel::GetTitle() const
	{
		return "Content Browser";
	}

	void ContentBrowserPanel::Draw()
	{
		auto window = DrawDockedWindow();
		if (window.IsVisible())
		{
			if (ImGui::BeginChild("##ContentBrowserBox", ImVec2(), ImGuiChildFlags_Borders))
			{
				int rows = *Editor::GetLiveSettings().content_browser->rows;
				ImGui::InputInt("Rows", &rows);
				*Editor::GetLiveSettings().content_browser->rows = std::max(rows, 1);

				// TODO v9.2
			}

			ImGui::EndChild();
		}
	}

	void ContentBrowserPanel::ShowInContentBrowser(const detail::ResourcePath& path)
	{
		// TODO v9.2
	}
}
