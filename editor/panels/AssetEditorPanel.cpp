#include "AssetEditorPanel.h"

#include <imgui.h>

#include "core/MainWindow.h"
#include "panels/PanelManager.h"

#include "documents/DocumentManager.h"
#include "documents/IDocument.h"

namespace oly::editor
{
	AssetEditorPanel& AssetEditorPanel::Instance()
	{
		auto panel = MainWindow::Instance().GetPanelManager().Get<AssetEditorPanel>();
		if (panel)
			return *panel;
		else
		{
			// TODO v7 error system for editor that prevents crashes. use new error codes
			throw std::runtime_error("No instance of AssetEditorPanel");
		}
	}

	const char* AssetEditorPanel::GetTitle() const
	{
		return "Asset Editor";
	}

	void AssetEditorPanel::Draw()
	{
		ImGui::Begin(GetTitle(), nullptr, ImGuiWindowFlags_MenuBar);

		ImGuiTabBarFlags tab_bar_flags =
			ImGuiTabBarFlags_AutoSelectNewTabs |
			ImGuiTabBarFlags_DrawSelectedOverline |
			ImGuiTabBarFlags_Reorderable;

		if (ImGui::BeginTabBar("AssetTabs", tab_bar_flags))
		{
			std::vector<size_t> closed;

			_selected_tab = nullptr;
			for (size_t i = 0; i < DocumentManager::Instance().DocumentCount(); ++i)
			{
				IDocument& doc = DocumentManager::Instance().GetDocument(i);
				bool open = true;

				ImGuiTabItemFlags tab_item_flags = 0;
				if (doc.IsDirty())
					tab_item_flags |= ImGuiTabItemFlags_UnsavedDocument;

				if (_focused_tab == &doc)
					tab_item_flags |= ImGuiTabItemFlags_SetSelected;

				std::string tabname = doc.GetOlyPath().tabname();
				if (ImGui::BeginTabItem((tabname + "##" + std::to_string(i)).c_str(), &open, tab_item_flags))
				{
					doc.Draw();
					_selected_tab = &doc;
					ImGui::EndTabItem();
				}

				if (!open)
					closed.push_back(i);
			}

			// TODO v7 prompt for unsaved changes
			for (auto it = closed.rbegin(); it != closed.rend(); ++it)
				DocumentManager::Instance().Remove(*it);

			_focused_tab = nullptr;
			ImGui::EndTabBar();
		}

		ImGui::End();
	}

	void AssetEditorPanel::FocusTab(IDocument* doc)
	{
		_focused_tab = doc;
	}

	bool AssetEditorPanel::IsSelected(IDocument* doc) const
	{
		return _selected_tab == doc;
	}

	void AssetEditorPanel::SaveSelectedTab() const
	{
		if (_selected_tab)
			_selected_tab->Dump();
	}

	void AssetEditorPanel::SaveAllTabs() const
	{
		for (size_t i = 0; i < DocumentManager::Instance().DocumentCount(); ++i)
			DocumentManager::Instance().GetDocument(i).Dump();
	}
}
