#include "AssetEditorPanel.h"

#include <imgui.h>

#include "documents/DocumentManager.h"
#include "documents/IDocument.h"

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

		for (size_t i = 0; i < DocumentManager::Instance().DocumentCount(); ++i)
		{
			IDocument& doc = DocumentManager::Instance().GetDocument(i);
			bool open = true;

			ImGuiTabItemFlags tab_item_flags = 0;
			if (doc.IsDirty())
				tab_item_flags |= ImGuiTabItemFlags_UnsavedDocument;

			if (ImGui::BeginTabItem(doc.GetTitle().c_str(), &open, tab_item_flags))
			{
				doc.Draw();
				ImGui::EndTabItem();
			}

			if (!open)
				closed.push_back(i);
		}

		for (auto it = closed.rbegin(); it != closed.rend(); ++it)
			DocumentManager::Instance().Remove(*it);

		ImGui::EndTabBar();
	}

	ImGui::End();
}
