#include "ContentBrowserPanel.h"

#include "core/Errors.h"
#include "core/PathInfo.h"

#include "core/editor/Editor.h"
#include "core/editor/LiveSettings.h"
#include "core/editor/Logger.h"
#include "core/editor/Notifier.h"
#include "core/editor/ProjectInfo.h"
#include "core/editor/ResourceLoader.h"
#include "core/editor/UID.h"

#include "core/windows/MainWindow.h"

#include "panels/PanelManager.h"
#include "panels/TreeViewPanel.h"

#include "gui/Controls.h"
#include "gui/ImGuiWrapper.h"
#include "gui/graphics/Toolbar.h"

#include "fio/FIOOperation.h"

#include "desc/impl/PreferencesDesc.h"

#include "definitions/Keys.h"

namespace oly::editor
{
	ContentBrowserPanel::NewAssetInfo::NewAssetInfo(detail::Key type, std::string name, const char* popup)
		: type(type), name(std::move(name)), popup(popup)
	{
	}

	ContentBrowserPanel::ContentBrowserPanel()
		: _folder_history(Editor::GetPreferences().content_browser.folder_history_limit.value)
	{
		_listener = Editor::Instance().OnPreferencesChanged.subscribe([this]() { _folder_history.set_limit(
			Editor::GetPreferences().content_browser.folder_history_limit.value
		); });
	}

	ContentBrowserPanel& ContentBrowserPanel::Instance()
	{
		if (auto panel = MainWindow::Instance().GetPanelManager().Get<ContentBrowserPanel>())
			return *panel;
		else
			BreakoutError::Throw("No instance of ContentBrowserPanel");
	}

	void ContentBrowserPanel::InitImpl()
	{
		_folder_history.clear();
		const auto res_root = ProjectInfo::Instance().ResourceRoot();
		GetFavoritesList().insert(res_root);
		SetFolder(res_root);
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
			gui::IDScope scope(this);

			if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
			{
				if (ImGui::Shortcut(ImGuiKey_Z | ImGuiMod_Ctrl, ImGuiInputFlags_RouteGlobal | ImGuiInputFlags_Repeat))
					_undo_history.Undo();

				if (ImGui::Shortcut(ImGuiKey_Z | ImGuiMod_Ctrl | ImGuiMod_Shift, ImGuiInputFlags_RouteGlobal | ImGuiInputFlags_Repeat))
					_undo_history.Redo();
			}

			if (ImGui::BeginChild("##ContentBrowserBox", ImVec2(0, 0), ImGuiChildFlags_Borders))
			{
				if (ImGui::BeginTable("##ContentBrowserToolbar", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV))
				{
					ImGui::TableNextRow();

					ImGui::TableSetColumnIndex(0);
					std::string preview = detail::ResourcePath(_folder).get_resource_shorthand();
					ImGui::SetNextItemWidth(ImGui::CalcTextSize(preview.c_str()).x + 10.f);
					ImGui::InputText("##Folder", preview.data(), preview.size() + 1, ImGuiInputTextFlags_ReadOnly);

					ImGui::TableSetColumnIndex(1);
					DrawMainToolbar();

					ImGui::EndTable();
				}

				const float font_global_scale = ImGui::GetIO().FontGlobalScale;
				ImGui::GetIO().FontGlobalScale *= *Editor::GetLiveSettings().content_browser->font_scale;

				std::vector<std::unique_ptr<UndoAction>> fio_operations;

				if (ImGui::BeginTable("##Table", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable))
				{
					ImGui::TableSetupColumn("##Favorites", ImGuiTableColumnFlags_WidthStretch, 0.25f);

					ImGui::TableNextRow();

					ImGui::TableSetColumnIndex(0);
					DrawFavoritesList();

					ImGui::TableSetColumnIndex(1);
					DrawFolderView(fio_operations);
					
					ImGui::EndTable();
				}

				for (auto& action : fio_operations)
					_undo_history.Execute(std::move(action));

				ImGui::GetIO().FontGlobalScale = font_global_scale;

				DrawNewAssetPopups();
			}

			ImGui::EndChild();
		}
	}

	ContentBrowserPanel& ContentBrowserPanel::FocusInstance()
	{
		ContentBrowserPanel& panel = Instance();
		panel.Open();
		panel.GainFocus();
		return panel;
	}

	void ContentBrowserPanel::ShowInContentBrowser(const detail::ResourcePath& path)
	{
		if (path.is_resource())
		{
			if (path.is_directory())
				FocusInstance().SetFolder(path.get_absolute());
			else
				FocusInstance().SetFolder(path.get_absolute().parent_path());
		}
		else
			Notifier::NotifyError("\"" + path.string() + "\" is not located in the project resource folder");
	}

	void ContentBrowserPanel::ShowInContentBrowser(const std::filesystem::path& path)
	{
		if (detail::ResourcePath(path).is_resource())
		{
			if (std::filesystem::is_directory(path))
				FocusInstance().SetFolder(path);
			else
				FocusInstance().SetFolder(path.parent_path());
		}
		else
			Notifier::NotifyError("\"" + path.generic_string() + "\" is not located in the project resource folder");
	}

	void ContentBrowserPanel::DrawMainToolbar()
	{
		if (auto disabled = DisabledSection(_on_res_root))
		{
			if (Toolbar::DrawIconToggleButton(IconResource::StarFilled, IconResource::StarOutline, _favorited,
				disabled.Disabled() ? "Favorite (disabled for root folder)" : "Favorite"))
			{
				if (!disabled.Disabled())
					SyncFavoritesList();
			}
		}

		ImGui::SameLine();
		if (Toolbar::DrawIconButton(IconResource::FolderOpen, "Open in tree view", "##OpenInTreeView"))
			TreeViewPanel::ShowResourceFolderInTreeView(_folder);

		ImGui::SameLine();
		static const char* NEW_ASSET_POPUP = "New asset";
		if (Toolbar::DrawIconButton(IconResource::CirclePlus, "New asset", "##NewAsset"))
			ImGui::OpenPopup(NEW_ASSET_POPUP);

		if (ImGui::BeginPopup(NEW_ASSET_POPUP))
		{
			NewAssetMenu();
			ImGui::EndPopup();
		}

		gui::VerticalSeparator();

		if (auto d = DisabledSection(_folder_history.empty_backwards()))
		{
			if (Toolbar::DrawIconButton(IconResource::CircleLeft, "Back", "##FolderHistoryBack"))
			{
				_folder_history.move_backward();
				if (auto f = _folder_history.get_present())
					SwitchFolder(*f);
			}
		}

		ImGui::SameLine();

		if (auto d = DisabledSection(_folder_history.empty_forwards()))
		{
			if (Toolbar::DrawIconButton(IconResource::CircleRight, "Forward", "##FolderHistoryForward"))
			{
				_folder_history.move_forward();
				if (auto f = _folder_history.get_present())
					SwitchFolder(*f);
			}
		}

		gui::VerticalSeparator();

		int columns = *Editor::GetLiveSettings().content_browser->columns;
		ImGui::SetNextItemWidth(100.f);
		ImGui::InputInt("Columns", &columns);
		*Editor::GetLiveSettings().content_browser->columns = std::max(columns, 1);

		gui::VerticalSeparator();

		gui::FloatControl("Font scale", *Editor::GetLiveSettings().content_browser->font_scale, 120.f, 0.1f, 10.f, "%.1f", true);
	}

	void ContentBrowserPanel::SetFolder(std::filesystem::path folder)
	{
		SwitchFolder(std::move(folder));
		_folder_history.push(_folder);
	}

	void ContentBrowserPanel::SwitchFolder(std::filesystem::path folder)
	{
		_folder = std::move(folder);
		_favorited = ShouldBeFavorited();
		_on_res_root = std::filesystem::equivalent(_folder, ProjectInfo::Instance().ResourceRoot());
		_selected_path.reset(); // TODO v9.2 store vector of selected paths: multi-select
	}

	std::set<detail::ResourcePath>& ContentBrowserPanel::GetFavoritesList() const
	{
		return *Editor::GetLiveSettings().content_browser->favorites;
	}

	bool ContentBrowserPanel::ShouldBeFavorited() const
	{
		return GetFavoritesList().contains(_folder);
	}

	void ContentBrowserPanel::SyncFavoritesList() const
	{
		if (_favorited)
			GetFavoritesList().insert(_folder);
		else
			GetFavoritesList().erase(_folder);
	}

	void ContentBrowserPanel::DrawFavoritesList()
	{
		const float label_max_width = ImGui::GetContentRegionAvail().x;
		std::optional<detail::ResourcePath> open_folder;
		for (const auto& favorite : GetFavoritesList())
		{
			std::string label = favorite.get_resource_shorthand().substr(2); // remove '@/'
			if (label == ".")
				label = "@";

			FitPathLabel(label, label_max_width);
			if (ImGui::Selectable(label.c_str()))
				open_folder = favorite;
		}

		if (open_folder)
			ShowInContentBrowser(*open_folder);
	}

	void ContentBrowserPanel::DrawFolderView(std::vector<std::unique_ptr<UndoAction>>& fio_operations)
	{
		if (ImGui::BeginChild("##FolderView", ImVec2(0, 0), ImGuiChildFlags_Borders))
		{
			auto payload = ImGui::GetDragDropPayload();
			if (payload && payload->IsDataType(StringID(UID::PathDrag)))
			{
				ImGui::Button("Show in content browser", ImGui::GetContentRegionAvail());

				if (ImGui::BeginDragDropTarget())
				{
					if (auto payload = ImGui::AcceptDragDropPayload(StringID(UID::PathDrag)))
						ShowInContentBrowser(std::filesystem::path(std::string_view(reinterpret_cast<const char*>(payload->Data), payload->DataSize)));

					ImGui::EndDragDropTarget();
				}
			}
			else
			{
				if (ImGui::BeginPopupContextWindow())
				{
					if (ImGui::BeginMenu("New asset"))
					{
						NewAssetMenu();
						ImGui::EndMenu();
					}
					ImGui::EndPopup();
				}

				DrawPathTable(fio_operations);
			}
		}

		ImGui::EndChild();
	}

	void ContentBrowserPanel::DrawPathTable(std::vector<std::unique_ptr<UndoAction>>& fio_operations)
	{
		const unsigned int columns = *Editor::GetLiveSettings().content_browser->columns;
		if (ImGui::BeginTable("##PathEntryTable", columns, ImGuiTableFlags_SizingFixedSame))
		{
			const float full_width = ImGui::GetContentRegionAvail().x - columns * 2 * ImGui::GetStyle().CellPadding.x;
			const float width = full_width / columns;
			const ImVec2 path_entry_size(width, width);

			ImGui::TableNextRow();

			const std::filesystem::path folder = _folder;

			if (!_on_res_root)
			{
				ImGui::TableNextColumn();
				DrawPathEntry(folder.parent_path(), true, path_entry_size, fio_operations);
			}

			std::error_code ec;
			for (const auto& entry : std::filesystem::directory_iterator(folder, std::filesystem::directory_options::skip_permission_denied, ec))
			{
				ImGui::TableNextColumn();
				DrawPathEntry(entry.path(), false, path_entry_size, fio_operations);
			}

			ImGui::EndTable();
		}

		if (ImGui::IsWindowHovered())
		{
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				_selected_path.reset();
		}

		if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows))
		{
			const float scroll = ImGui::GetIO().MouseWheel;
			if (scroll != 0.f)
			{
				int columns = *Editor::GetLiveSettings().content_browser->columns;
				columns = std::max(columns - static_cast<int>(scroll), 1);
				*Editor::GetLiveSettings().content_browser->columns = columns;
			}
		}
	}

	void ContentBrowserPanel::DrawPathEntry(const std::filesystem::path& path, bool dotdot, const ImVec2 size, std::vector<std::unique_ptr<UndoAction>>& fio_operations)
	{
		// TODO v9.2 drag-n-drop into documents, just like TreeView

		if (ImGui::BeginChild(path.generic_string().c_str(), size, ImGuiChildFlags_Borders))
		{
			static constexpr const char* RENAME_POPUP = "Rename path";
			bool open_rename_popup = false;

			if (!dotdot)
			{
				if (ImGui::BeginPopupContextWindow())
				{
					if (ImGui::MenuItem("Open"))
						OpenPath(path);

					if (ImGui::MenuItem("Rename", "F2"))
						open_rename_popup = true;

					if (ImGui::MenuItem("Delete"))
						DeletePath(path, fio_operations);

					// TODO v9.2 check if path is an importable asset or a folder -> context menu to import. Also options to prune (remove unused import files)

					ImGui::EndPopup();
				}
			}

			std::string label = dotdot ? ".." : path.filename().generic_string();
			
			const ImVec2 padding_offset = ImGui::GetStyle().CellPadding + ImGui::GetStyle().WindowPadding;
			const ImVec2 cursor = ImGui::GetCursorScreenPos();
			const ImVec2 child_size = ImGui::GetContentRegionAvail();
			
			const ImVec2 label_size = FitPathLabel(label, child_size.x);
			const ImVec2 label_offset = (child_size - label_size) * ImVec2(0.5f, 1.f);
			
			const ImVec2 icon_size = child_size - ImVec2(label_size.y, label_size.y);
			const ImVec2 icon_start = cursor + ImVec2(0.5f * (child_size.x - icon_size.x), 0.f);

			if (_selected_path == path)
			{
				ImGui::GetWindowDrawList()->AddRectFilled(cursor - padding_offset, cursor + child_size + 2 * padding_offset,
					ImGui::GetColorU32(ImGuiCol_FrameBgActive));
				ImGui::GetWindowDrawList()->AddRect(cursor - padding_offset, cursor + child_size + 2 * padding_offset,
					ImGui::GetColorU32(ImGuiCol_TabSelectedOverline), 0.f, 0, 3.f);
			}

			ImGui::SetCursorScreenPos(cursor + label_offset);
			ImGui::TextUnformatted(label.c_str());

			if (ImGui::IsWindowHovered())
			{
				ImGui::SetTooltip(detail::ResourcePath(path).get_resource_shorthand().c_str());

				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					_selected_path = path;

				ImGui::GetWindowDrawList()->AddRectFilled(cursor - padding_offset, cursor + child_size + 2 * padding_offset, ImGui::GetColorU32(ImGuiCol_FrameBgHovered));

				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					OpenPath(path);
			}

			ImGui::GetWindowDrawList()->AddImage(PathInfo::GetIcon(path).ID(), icon_start, icon_start + icon_size);

			if (ImGui::IsWindowFocused() && _selected_path == path && !dotdot)
			{
				if (ImGui::Shortcut(ImGuiKey_Enter, ImGuiInputFlags_RouteGlobal))
					OpenPath(path);

				if (ImGui::Shortcut(ImGuiKey_F2, ImGuiInputFlags_RouteGlobal))
					open_rename_popup = true;

				if (ImGui::Shortcut(ImGuiKey_Delete, ImGuiInputFlags_RouteGlobal))
					DeletePath(path, fio_operations);

				// TODO v9.2 FIO operations: ctrl+c, ctrl+x, ctrl+v, etc.
			}

			if (open_rename_popup)
			{
				_rename_buffer = path.filename().generic_string();
				ImGui::OpenPopup(RENAME_POPUP);
			}

			ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			if (ImGui::BeginPopupModal(RENAME_POPUP, 0, ImGuiWindowFlags_AlwaysAutoResize))
			{
				gui::InputText("Filename", _rename_buffer);

				if (ImGui::Shortcut(ImGuiKey_Escape))
					ImGui::CloseCurrentPopup();

				if (ImGui::Shortcut(ImGuiKey_Enter))
				{
					ImGui::CloseCurrentPopup();

					auto action = std::make_unique<fio::RenamePathAction>();
					action->old_path = path;
					action->new_path = path.parent_path() / _rename_buffer;
					fio_operations.push_back(std::move(action));
				}

				ImGui::EndPopup();
			}
		}

		ImGui::EndChild();
	}

	ImVec2 ContentBrowserPanel::FitPathLabel(std::string& label, const float width)
	{
		ImVec2 label_size = ImGui::CalcTextSize(label.c_str());
		if (width > 0.f && label_size.x > width)
		{
			static constexpr const char* ellipses = "...";
			const ImVec2 ellipses_size = ImGui::CalcTextSize(ellipses);

			while (label_size.x + ellipses_size.x > width)
			{
				label.pop_back();
				if (label.empty())
				{
					label = ellipses;
					label_size = ellipses_size;
					break;
				}

				label_size = ImGui::CalcTextSize(label.c_str());
			}

			label += ellipses;
			label_size.x += ellipses_size.x;
			label_size.y = std::max(label_size.y, ellipses_size.y);
		}
		return label_size;
	}

	void ContentBrowserPanel::OpenPath(const std::filesystem::path& path)
	{
		if (std::filesystem::is_directory(path))
			ShowInContentBrowser(path);
		else
			Editor::Instance().OpenFile(path);
	}

	void ContentBrowserPanel::NewAssetMenu()
	{
		// TODO v9.2 icons for all assets

		if (Toolbar::IconMenuItem("Tileset", IconResource::File))
		{
			_new_asset = NewAssetInfo(detail::Key::Meta_Tileset, "New Tileset", "New tileset");
			ImGui::CloseCurrentPopup();
		}

		if (Toolbar::IconMenuItem("Signal", IconResource::Controller))
		{
			_new_asset = NewAssetInfo(detail::Key::Meta_Signal, "New Signal", "New signal");
			ImGui::CloseCurrentPopup();
		}

		if (ImGui::BeginMenu("Fonts"))
		{
			if (Toolbar::IconMenuItem("Font family", IconResource::File))
			{
				_new_asset = NewAssetInfo(detail::Key::Meta_FontFamily, "New Font Family", "New font family");
				ImGui::CloseCurrentPopup();
			}

			if (Toolbar::IconMenuItem("Raster font", IconResource::File))
			{
				_new_asset = NewAssetInfo(detail::Key::Meta_RasterFont, "New Raster Font", "New raster font");
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndMenu();
		}
	}

	void ContentBrowserPanel::DrawNewAssetPopups()
	{
		if (!_new_asset)
			return;

		if (_new_asset->pending_popup)
		{
			ImGui::OpenPopup(_new_asset->popup);
			_new_asset->pending_popup = false;
		}

		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal(_new_asset->popup, 0, ImGuiWindowFlags_AlwaysAutoResize))
		{
			gui::InputText("Filename", _new_asset->name);

			if (ImGui::Button("Create"))
			{
				// TODO v9.2 fio operation to create new file -> load with initial data using Document. If filename exists, use (1)/(2)/etc. Only then push fio operation to undo stack.
				_new_asset.reset();
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel"))
			{
				_new_asset.reset();
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::Shortcut(ImGuiKey_Escape))
			{
				_new_asset.reset();
				ImGui::CloseCurrentPopup();
			}
			
			ImGui::EndPopup();
		}
	}

	void ContentBrowserPanel::DeletePath(const std::filesystem::path& path, std::vector<std::unique_ptr<UndoAction>>& fio_operations) const
	{
		detail::ResourcePath resource = path;

		auto action = std::make_unique<fio::DeletePathAction>();
		action->del_path = resource;

		if (!resource.is_import_path())
		{
			detail::ResourcePath import = resource.get_import_path();
			if (PathInfo::IsImportFile(import.get_absolute()))
				action->aux_path = import;
		}

		fio_operations.push_back(std::move(action));
	}
}
