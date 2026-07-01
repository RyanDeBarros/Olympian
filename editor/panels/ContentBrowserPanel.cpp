#include "ContentBrowserPanel.h"

#include "core/Errors.h"

#include "core/editor/Editor.h"
#include "core/editor/LiveSettings.h"
#include "core/editor/Logger.h"
#include "core/editor/ProjectInfo.h"

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
		_folder = ProjectInfo::Instance().ResourceRoot();
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
			if (ImGui::BeginChild("##ContentBrowserBox", ImVec2(0, 0), ImGuiChildFlags_Borders))
			{
				int rows = *Editor::GetLiveSettings().content_browser->rows;
				ImGui::InputInt("Rows", &rows);
				*Editor::GetLiveSettings().content_browser->rows = std::max(rows, 1);

				// TODO v9.2 toolbar for '<'/'>' (keep stack of folder history so as to go back and forth between folders), favorites button (pop out modal with list of favorites / star button to toggle favorite status of current folder / etc.), etc.

				DrawFolderView();
			}

			ImGui::EndChild();
		}
	}

	void ContentBrowserPanel::ShowInContentBrowser(const detail::ResourcePath& path)
	{
		if (path.is_resource())
			_folder = path.get_absolute();
		else
			MainWindow::Instance().PushNotification(Notification(LogLevel::Error, path.string() + " is not located in the project resource folder"));
	}

	void ContentBrowserPanel::DrawFolderView()
	{
		if (ImGui::BeginChild("##FolderView", ImVec2(0, 0), ImGuiChildFlags_Borders))
		{
			const unsigned int rows = *Editor::GetLiveSettings().content_browser->rows;
			if (ImGui::BeginTable("##PathEntryTable", rows))
			{
				ImGui::TableNextRow();

				// TODO v9.2 draw _folder content. Use ImGui::TableNextColumn() *only*, don't worry about row/col indexes.

				ImGui::EndTable();
			}
		}

		ImGui::EndChild();
	}
}
