#include "AssetEditorPanel.h"

#include "core/MainWindow.h"
#include "core/Logger.h"
#include "core/Editor.h"
#include "core/Errors.h"
#include "core/ProjectInfo.h"

#include "panels/PanelManager.h"
#include "documents/DocumentManager.h"
#include "documents/IDocument.h"

#include <imgui.h>
#include <ImGuiFileDialog.h>

namespace oly::editor
{
	static const char* OPEN_FILE = "OpenFileDlg";

	static std::string open_file_parent = ".";

	AssetEditorPanel& AssetEditorPanel::Instance()
	{
		if (auto panel = MainWindow::Instance().GetPanelManager().Get<AssetEditorPanel>())
			return *panel;
		else
			BreakoutError::Throw("No instance of AssetEditorPanel");
	}

	void AssetEditorPanel::Init()
	{
		open_file_parent = ProjectInfo::Instance().ResourceRoot().generic_string();
	}

	const char* AssetEditorPanel::GetTitle() const
	{
		return "Asset Editor";
	}

	void AssetEditorPanel::Draw()
	{
		auto window = DrawDockedWindow(ImGuiWindowFlags_MenuBar);
		if (window.IsVisible())
		{
			PollShortcuts();

			DrawTabBar();

			if (_selected_tab)
				_selected_tab->DrawMenuBar();
			else
				DrawDefaultMenuBar();

			ImGui::SetNextWindowSize(ImGui::GetMainViewport()->WorkSize * 0.5f, ImGuiCond_FirstUseEver);

			if (ImGuiFileDialog::Instance()->Display(OPEN_FILE, ImGuiWindowFlags_NoCollapse))
			{
				if (ImGuiFileDialog::Instance()->IsOk())
				{
					std::filesystem::path path = ImGuiFileDialog::Instance()->GetFilePathName();
					open_file_parent = path.parent_path().string();
					Editor::Instance().OpenFile(path);
				}

				ImGuiFileDialog::Instance()->Close();
			}
		}
	}

	void AssetEditorPanel::PollShortcuts()
	{
		if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_O, ImGuiInputFlags_RouteGlobal))
			OpenFile();

		if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_S, ImGuiInputFlags_RouteGlobal))
			SaveSelectedTab();

		if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_S, ImGuiInputFlags_RouteGlobal))
			SaveAllTabs();
	}

	void AssetEditorPanel::DrawTabBar()
	{
		ImGuiTabBarFlags tab_bar_flags =
			ImGuiTabBarFlags_AutoSelectNewTabs |
			ImGuiTabBarFlags_DrawSelectedOverline |
			ImGuiTabBarFlags_Reorderable;

		if (ImGui::BeginTabBar("##AssetTabs", tab_bar_flags))
		{
			std::vector<size_t> closed;

			_selected_tab = nullptr;
			std::unordered_set<IDocument*> seen_documents;

			for (size_t i = 0; i < DocumentManager::Instance().DocumentCount(); ++i)
			{
				IDocument& doc = DocumentManager::Instance().GetDocument(i);
				seen_documents.insert(&doc);
				bool open = true;

				ImGuiTabItemFlags tab_item_flags = 0;
				if (doc.IsDirty())
					tab_item_flags |= ImGuiTabItemFlags_UnsavedDocument;

				if (_focused_tab == &doc)
					tab_item_flags |= ImGuiTabItemFlags_SetSelected;

				if (ImGui::BeginTabItem((doc.TabName() + "##" + std::to_string(i)).c_str(), &open, tab_item_flags))
				{
					doc.Draw();
					_selected_tab = &doc;
					ImGui::EndTabItem();
				}

				if (!open)
				{
					if (!doc.IsDirty())
						closed.push_back(i);
					else if (!_pending_close_set.contains(&doc))
					{
						_pending_close_set.insert(&doc);
						_pending_close.push_back(&doc);
						ImGui::OpenPopup("Unsaved Changes");
					}
				}
			}

			RemoveOldPendingDocuments(seen_documents);
			DrawUnsavedChangesModal(closed);

			std::sort(closed.begin(), closed.end());
			for (auto it = closed.rbegin(); it != closed.rend(); ++it)
				DocumentManager::Instance().Remove(*it);

			_focused_tab = nullptr;
			ImGui::EndTabBar();
		}
	}

	void AssetEditorPanel::RemoveOldPendingDocuments(const std::unordered_set<IDocument*> seen_documents)
	{
		for (auto it = _pending_close.begin(); it != _pending_close.end();)
		{
			if (seen_documents.contains(*it))
				++it;
			else
				it = _pending_close.erase(it);
		}

		for (auto it = _pending_close_set.begin(); it != _pending_close_set.end();)
		{
			if (seen_documents.contains(*it))
				++it;
			else
				it = _pending_close_set.erase(it);
		}
	}

	void AssetEditorPanel::DrawUnsavedChangesModal(std::vector<size_t>& closed)
	{
		if (ImGui::BeginPopupModal("Unsaved Changes", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			IDocument* doc = _pending_close.front();

			ImGui::Text(("Asset " + doc->TabName()).c_str());
			ImGui::Text(("Full path: " + doc->GetOlyPath().string()).c_str());

			if (ImGui::Button("Save Changes"))
			{
				doc->Dump();
				closed.push_back(DocumentManager::Instance().GetDocumentIndex(doc));
				_pending_close.erase(_pending_close.begin());
				_pending_close_set.erase(doc);
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();
			if (ImGui::Button("Discard Changes"))
			{
				doc->Load();
				closed.push_back(DocumentManager::Instance().GetDocumentIndex(doc));
				_pending_close.erase(_pending_close.begin());
				_pending_close_set.erase(doc);
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();
			if (ImGui::Button("Cancel Close"))
			{
				_pending_close.erase(_pending_close.begin());
				_pending_close_set.erase(doc);
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	void AssetEditorPanel::DrawDefaultMenuBar()
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open File", "Ctrl+O"))
					OpenFile();

				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
	}

	void AssetEditorPanel::OpenFile()
	{
		IGFD::FileDialogConfig config;
		config.path = open_file_parent;
		config.flags = ImGuiFileDialogFlags_CaseInsensitiveExtentionFiltering | ImGuiFileDialogFlags_Modal;

		ImGuiFileDialog::Instance()->OpenDialog(OPEN_FILE, "Select File", "Olympian files (*.oly){.oly},Image files (*.png, *.jpg, *.gif){.png,.jpg,.gif},Font files (*.ttf, *.otf){.ttf,.otf},Any files{.*}", config);
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
