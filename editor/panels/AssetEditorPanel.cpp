#include "AssetEditorPanel.h"

#include "core/windows/MainWindow.h"
#include "core/editor/Logger.h"
#include "core/editor/Editor.h"
#include "core/editor/ProjectInfo.h"
#include "core/PathInfo.h"
#include "core/Errors.h"

#include "panels/PanelManager.h"
#include "documents/DocumentManager.h"
#include "documents/IDocument.h"

#include "gui/scopes/IDScope.h"

#include "assets/MetaSplitter.h"

#include <imgui.h>
#include <ImGuiFileDialog.h>

namespace oly::editor
{
	static constexpr const char* kTabUnsavedChangesPopup = "Unsaved Changes##Tab";
	static constexpr const char* kWindowUnsavedChangesPopup = "Unsaved Changes##Window";
	static constexpr const char* kShutdownUnsavedChangesPopup = "Unsaved Changes##App";
	static constexpr const char* kOpenFile = "OpenFileDlg";

	static std::string open_file_parent = ".";

	AssetEditorPanel& AssetEditorPanel::Instance()
	{
		if (auto panel = MainWindow::Instance().GetPanelManager().Get<AssetEditorPanel>())
			return *panel;
		else
			BreakoutError::Throw("No instance of AssetEditorPanel");
	}

	void AssetEditorPanel::InitImpl()
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
		bool draw_window = true;
		if (window.RequestsClose())
		{
			CloseAllTabs(_window_unsaved_changes_modal);
			if (_window_unsaved_changes_modal)
			{
				Open();
				ImGui::SetWindowFocus();
				ImGui::OpenPopup(kWindowUnsavedChangesPopup);
			}
			else
				draw_window = false;
		}

		if (draw_window && window.IsVisible())
		{
			PollShortcuts();

			DrawTabBar();

			if (_selected_tab)
				_selected_tab->DrawMenuBar();
			else
				DrawDefaultMenuBar();

			ImGui::SetNextWindowSize(ImGui::GetMainViewport()->WorkSize * 0.5f, ImGuiCond_FirstUseEver);

			if (ImGuiFileDialog::Instance()->Display(kOpenFile, ImGuiWindowFlags_NoCollapse))
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

		if (_window_unsaved_changes_modal)
		{
			if (DrawUnsavedChangesModal(_window_unsaved_changes_modal, kWindowUnsavedChangesPopup))
				Close();
		}

		if (_shutdown_unsaved_changes_modal)
		{
			if (_open_shutdown_modal)
			{
				_open_shutdown_modal = false;
				ImGui::OpenPopup(kShutdownUnsavedChangesPopup);
			}

			if (DrawUnsavedChangesModal(_shutdown_unsaved_changes_modal, kShutdownUnsavedChangesPopup))
			{
				Close();
				Editor::Instance().RequestShutdown();
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

		if (ImGui::Shortcut(ImGuiKey_Z | ImGuiMod_Ctrl, ImGuiInputFlags_RouteGlobal))
			SelectedTabUndo();

		if (ImGui::Shortcut(ImGuiKey_Z | ImGuiMod_Ctrl | ImGuiMod_Shift, ImGuiInputFlags_RouteGlobal))
			SelectedTabRedo();
	}

	void AssetEditorPanel::DrawTabBar()
	{
		ImGuiTabBarFlags tab_bar_flags =
			ImGuiTabBarFlags_AutoSelectNewTabs |
			ImGuiTabBarFlags_DrawSelectedOverline |
			ImGuiTabBarFlags_Reorderable;

		gui::IDScope scope(this);
		if (ImGui::BeginTabBar("##AssetTabs", tab_bar_flags))
		{
			std::vector<size_t> closed;

			_selected_tab = nullptr;
			std::unordered_set<IDocument*> seen_documents;

			for (size_t i = 0; i < DocumentManager::Instance().DocumentCount(); ++i)
			{
				scope.Push(i);
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
					if (ImGui::BeginPopupContextItem("ContextMenu"))
					{
						detail::ResourcePath path = doc.GetOlyPath();
						if (detail::MetaSplitter::decode_meta(path).is_import())
							path = path.get_source_path();

						if (ImGui::MenuItem("Show in Content Browser"))
						{
							// TODO v9.2
						}

						if (ImGui::MenuItem("Reveal in explorer"))
							PathInfo::RevealInExplorer(path.get_absolute());

						ImGui::EndPopup();
					}

					doc.Draw();
					_selected_tab = &doc;
					ImGui::EndTabItem();
				}

				if (!open)
				{
					if (!doc.IsDirty())
					{
						closed.push_back(i);
						if (&doc == _selected_tab)
							_selected_tab = nullptr;
					}
					else if (!_pending_close_set.contains(&doc))
					{
						_pending_close_set.insert(&doc);
						_pending_close.push_back(&doc);
						ImGui::OpenPopup(kTabUnsavedChangesPopup);
					}
				}
			}

			RemoveOldPendingDocuments(seen_documents);

			if (!_window_unsaved_changes_modal)
				DrawTabUnsavedChangesModal(closed);
			else
			{
				_pending_close.clear();
				_pending_close_set.clear();
			}

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

	// TODO v9.1 general utility for unsaved changes modal in gui/. That way any decorative changes can be made to one source
	enum class UnsavedChangesModalResult
	{
		None,
		SaveChanges,
		DiscardChanges,
		CancelClose
	};

	static UnsavedChangesModalResult DrawUnsavedChangesModalImpl(const char* popup, IDocument* doc)
	{
		UnsavedChangesModalResult result = UnsavedChangesModalResult::None;

		if (!doc)
			return result;

		if (ImGui::BeginPopupModal(popup, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text(("Asset " + doc->TabName()).c_str());
			ImGui::Text(("Full path: " + doc->GetOlyPath().string()).c_str());

			if (ImGui::Button("Save Changes"))
			{
				doc->Dump();
				ImGui::CloseCurrentPopup();
				result = UnsavedChangesModalResult::SaveChanges;
			}

			ImGui::SameLine();
			if (ImGui::Button("Discard Changes"))
			{
				doc->Load();
				ImGui::CloseCurrentPopup();
				result = UnsavedChangesModalResult::DiscardChanges;
			}

			ImGui::SameLine();
			if (ImGui::Button("Cancel Close"))
			{
				ImGui::CloseCurrentPopup();
				result = UnsavedChangesModalResult::CancelClose;
			}

			ImGui::EndPopup();
		}
		
		return result;
	}

	void AssetEditorPanel::DrawTabUnsavedChangesModal(std::vector<size_t>& closed)
	{
		IDocument* doc = _pending_close.empty() ? nullptr : _pending_close.front();
		switch (DrawUnsavedChangesModalImpl(kTabUnsavedChangesPopup, doc))
		{
		case UnsavedChangesModalResult::SaveChanges:
			closed.push_back(DocumentManager::Instance().GetDocumentIndex(doc));
			if (doc == _selected_tab)
				_selected_tab = nullptr;
			_pending_close.erase(_pending_close.begin());
			_pending_close_set.erase(doc);
			break;

		case UnsavedChangesModalResult::DiscardChanges:
			closed.push_back(DocumentManager::Instance().GetDocumentIndex(doc));
			if (doc == _selected_tab)
				_selected_tab = nullptr;
			_pending_close.erase(_pending_close.begin());
			_pending_close_set.erase(doc);
			break;

		case UnsavedChangesModalResult::CancelClose:
			_pending_close.erase(_pending_close.begin());
			_pending_close_set.erase(doc);
			break;
		}
	}

	bool AssetEditorPanel::DrawUnsavedChangesModal(bool& unsaved_changes_modal, const char* popup)
	{
		switch (DrawUnsavedChangesModalImpl(popup, _selected_tab))
		{
		case UnsavedChangesModalResult::SaveChanges:
		case UnsavedChangesModalResult::DiscardChanges:
			CloseAllTabs(unsaved_changes_modal);
			if (unsaved_changes_modal)
				ImGui::OpenPopup(popup);
			return !unsaved_changes_modal;

		case UnsavedChangesModalResult::CancelClose:
			unsaved_changes_modal = false;
			return false;

		default:
			return false;
		}
	}

	void AssetEditorPanel::CloseAllTabs(bool& unsaved_changes_modal)
	{
		_pending_close.clear();
		_pending_close_set.clear();
		unsaved_changes_modal = false;

		for (int i = DocumentManager::Instance().DocumentCount() - 1; i >= 0; --i)
		{
			IDocument& doc = DocumentManager::Instance().GetDocument(i);
			_selected_tab = &doc;

			if (doc.IsDirty())
			{
				unsaved_changes_modal = true;
				break;
			}

			DocumentManager::Instance().Remove(i);
			_selected_tab = nullptr;
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

		ImGuiFileDialog::Instance()->OpenDialog(kOpenFile, "Select File", "Olympian files (*.oly){.oly},Image files (*.png, *.jpg, *.gif){.png,.jpg,.gif},Font files (*.ttf, *.otf){.ttf,.otf},Any files{.*}", config);
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

	void AssetEditorPanel::SelectedTabUndo() const
	{
		if (_selected_tab)
			_selected_tab->Undo();
	}

	void AssetEditorPanel::SelectedTabRedo() const
	{
		if (_selected_tab)
			_selected_tab->Redo();
	}

	bool AssetEditorPanel::RequestShutdown()
	{
		CloseAllTabs(_shutdown_unsaved_changes_modal);
		if (_shutdown_unsaved_changes_modal)
		{
			Open();
			GainFocus();
			_open_shutdown_modal = true;
			return false;
		}
		else
			return true;
	}
}
