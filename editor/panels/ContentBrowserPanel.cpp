#include "ContentBrowserPanel.h"

#include "core/Colors.h"
#include "core/Errors.h"
#include "core/PathInfo.h"

#include "core/editor/Editor.h"
#include "core/editor/LiveSettings.h"
#include "core/editor/Logger.h"
#include "core/editor/Notifier.h"
#include "core/editor/ProjectInfo.h"
#include "core/editor/ResourceLoader.h"

#include "core/windows/MainWindow.h"

#include "panels/PanelManager.h"
#include "panels/TreeViewPanel.h"

#include "gui/ImGuiWrapper.h"
#include "gui/graphics/Toolbar.h"

#include "fio/FIOOperation.h"

#include "desc/impl/PreferencesDesc.h"

#include "definitions/Keys.h"

// TODO v9.4 options in editor filesystem settings to auto-prune/auto-import

namespace oly::editor
{
	ContentBrowserPanel::NewAssetInfo::NewAssetInfo(detail::Key type, std::string name, const char* popup_label)
		: type(type), name(std::move(name)), popup(popup_label, imtk::popup_config{ .center_window = imtk::center_window::always, .window_flags = ImGuiWindowFlags_AlwaysAutoResize })
	{
		popup.open();
	}

	ContentBrowserPanel::ImportFolderInfo::ImportFolderInfo(std::filesystem::path folder)
		: popup("Import folder"), folder(std::move(folder))
	{
		popup.open();
	}

	ContentBrowserPanel::PruneFolderInfo::PruneFolderInfo(std::filesystem::path folder)
		: popup("Prune folder"), folder(std::move(folder))
	{
		popup.open();
	}

	ContentBrowserPanel::ContentBrowserPanel()
		: _folder_history(Editor::GetPreferences().content_browser.folder_history_limit.value)
	{
		_listener = Editor::OnPreferencesChanged().subscribe([this]() { _folder_history.set_limit(
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
			imtk::id_scope scope(this);

			if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
			{
				if (ImGui::Shortcut(ImGuiKey_Z | ImGuiMod_Ctrl, ImGuiInputFlags_RouteGlobal | ImGuiInputFlags_Repeat))
					_undo_history.Undo();

				if (ImGui::Shortcut(ImGuiKey_Z | ImGuiMod_Ctrl | ImGuiMod_Shift, ImGuiInputFlags_RouteGlobal | ImGuiInputFlags_Repeat))
					_undo_history.Redo();
			}

			if (auto _ = imtk::child("##ContentBrowserBox", ImVec2(0, 0), ImGuiChildFlags_Borders))
			{
				CompoundUndoActionQueue fio_queue;

				if (auto _ = imtk::table("##ContentBrowserToolbar", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV))
				{
					ImGui::TableNextRow();

					ImGui::TableSetColumnIndex(0);
					std::string preview = detail::ResourcePath(_folder).get_resource_shorthand();
					ImGui::SetNextItemWidth(ImGui::CalcTextSize(preview.c_str()).x + 10.f);
					ImGui::InputText("##Folder", preview.data(), preview.size() + 1, ImGuiInputTextFlags_ReadOnly);

					ImGui::TableSetColumnIndex(1);
					DrawMainToolbar(fio_queue);
				}

				const float font_global_scale = ImGui::GetIO().FontGlobalScale;
				ImGui::GetIO().FontGlobalScale *= *Editor::GetLiveSettings().content_browser->font_scale;

				if (auto _ = imtk::table("##MainTable", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable))
				{
					ImGui::TableSetupColumn("##Favorites", ImGuiTableColumnFlags_WidthStretch, 0.25f);

					ImGui::TableNextRow();

					ImGui::TableSetColumnIndex(0);
					DrawFavoritesList();

					ImGui::TableSetColumnIndex(1);
					DrawFolderView(fio_queue);
				}

				ImGui::GetIO().FontGlobalScale = font_global_scale;

				DrawNewAssetPopups(fio_queue);
				DrawImportFolderPopup(fio_queue);
				DrawPruneFolderPopup(fio_queue);

				fio_queue.PushAll(_undo_history);
			}
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

	void ContentBrowserPanel::DrawMainToolbar(CompoundUndoActionQueue& fio_queue)
	{
		if (auto d = imtk::disabled(_on_res_root))
		{
			if (Toolbar::DrawIconToggleButton(IconResource::StarFilled, IconResource::StarOutline, _favorited,
				d.is_disabled() ? "Favorite (disabled for root folder)" : "Favorite"))
			{
				if (!d.is_disabled())
					SyncFavoritesList();
			}
		}

		ImGui::SameLine();
		if (Toolbar::DrawIconButton(IconResource::OpenInTreeView, "Open in tree view", "##OpenInTreeView"))
			TreeViewPanel::ShowResourceFolderInTreeView(_folder);

		ImGui::SameLine();
		if (Toolbar::DrawIconButton(IconResource::FolderOpen, "Reveal in explorer", "##RevealInExplorer"))
			PathInfo::RevealInExplorer(_folder, true);

		ImGui::SameLine();
		imtk::popup new_asset_popup("New");
		if (Toolbar::DrawIconButton(IconResource::CirclePlus, "New", "##New"))
			new_asset_popup.open();

		if (auto d = new_asset_popup.draw())
		{
			NewFolderMenu();
			ImGui::Separator();

			if (auto _ = imtk::menu("New asset"))
				NewAssetMenu();
		}

		imtk::controls::vertical_separator();

		if (Toolbar::DrawIconButton(IconResource::Import, "Import", "##Import"))
		{
			ImportFromPath(_folder, fio_queue);
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();
		if (Toolbar::DrawIconButton(IconResource::Prune, "Prune", "##Prune"))
		{
			PruneFromPath(_folder, fio_queue);
			ImGui::CloseCurrentPopup();
		}

		imtk::controls::vertical_separator();

		if (auto d = imtk::disabled(_folder_history.empty_backwards()))
		{
			if (Toolbar::DrawIconButton(IconResource::CircleLeft, "Back", "##FolderHistoryBack"))
			{
				_folder_history.move_backward();
				if (auto f = _folder_history.get_present())
					SwitchFolder(*f);
			}
		}

		ImGui::SameLine();

		if (auto d = imtk::disabled(_folder_history.empty_forwards()))
		{
			if (Toolbar::DrawIconButton(IconResource::CircleRight, "Forward", "##FolderHistoryForward"))
			{
				_folder_history.move_forward();
				if (auto f = _folder_history.get_present())
					SwitchFolder(*f);
			}
		}

		imtk::controls::vertical_separator();

		int columns = *Editor::GetLiveSettings().content_browser->columns;
		ImGui::SetNextItemWidth(100.f);
		ImGui::InputInt("Columns", &columns);
		*Editor::GetLiveSettings().content_browser->columns = std::max(columns, 1);

		imtk::controls::vertical_separator();

		if (auto _ = imtk::item_width_scope(120.f))
			imtk::float_control("Font scale", *Editor::GetLiveSettings().content_browser->font_scale, 0.1f, 10.f, "%.1f", ImGuiSliderFlags_Logarithmic);
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
		ClearSelection();
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

	void ContentBrowserPanel::DrawFolderView(CompoundUndoActionQueue& fio_queue)
	{
		if (auto _ = imtk::child("##FolderView", ImVec2(0, 0), ImGuiChildFlags_Borders))
		{
			if (imtk::drag_drop_is_type<TreeViewPathDDP>())
			{
				ImGui::Button("Show in content browser", ImGui::GetContentRegionAvail());

				if (auto target = imtk::drag_drop_target())
				{
					if (auto dropped_path = target.accept<TreeViewPathDDP>())
						ShowInContentBrowser(std::filesystem::path(*dropped_path));
				}
			}
			else
			{
				if (auto _ = imtk::context_menu::window())
				{
					NewFolderMenu();
					ImGui::Separator();

					if (auto _ = imtk::menu("New asset"))
						NewAssetMenu();

					ImGui::Separator();

					if (Toolbar::IconMenuItem("Import", IconResource::Import))
					{
						ImportFromPath(_folder, fio_queue);
						ImGui::CloseCurrentPopup();
					}

					if (Toolbar::IconMenuItem("Prune", IconResource::Prune))
					{
						PruneFromPath(_folder, fio_queue);
						ImGui::CloseCurrentPopup();
					}

					ImGui::Separator();

					if (Toolbar::IconMenuItem("Open in tree view", IconResource::OpenInTreeView))
					{
						TreeViewPanel::ShowResourceFolderInTreeView(_folder);
						ImGui::CloseCurrentPopup();
					}

					if (Toolbar::IconMenuItem("Reveal in explorer", IconResource::FolderOpen))
					{
						PathInfo::RevealInExplorer(_folder, true);
						ImGui::CloseCurrentPopup();
					}
				}

				DrawPathTable(fio_queue);
			}
		}
	}

	struct ContentBrowserPanel::EntryTableState
	{
		bool focused = false;
		ImVec2 entry_size;
		bool delete_consumed = false;
		bool enter_consumed = false;
	};

	void ContentBrowserPanel::DrawPathTable(CompoundUndoActionQueue& fio_queue)
	{
		PruneSelection();

		const unsigned int columns = *Editor::GetLiveSettings().content_browser->columns;
		if (auto _ = imtk::table("##PathEntryTable", columns, ImGuiTableFlags_SizingFixedSame))
		{
			EntryTableState entry_table_state;
			entry_table_state.focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);
			entry_table_state.delete_consumed = ImGui::Shortcut(ImGuiKey_Delete, ImGuiInputFlags_RouteGlobal);
			entry_table_state.enter_consumed = ImGui::Shortcut(ImGuiKey_Enter, ImGuiInputFlags_RouteGlobal);

			const float full_width = ImGui::GetContentRegionAvail().x - columns * 2 * ImGui::GetStyle().CellPadding.x;
			const float width = full_width / columns;
			entry_table_state.entry_size = ImVec2(width, width);

			ImGui::TableNextRow();

			const std::filesystem::path folder = _folder;

			if (!_on_res_root)
			{
				ImGui::TableNextColumn();
				DrawPathEntry(folder.parent_path(), true, entry_table_state, fio_queue);
			}

			std::error_code ec;
			for (const auto& path : _selectable_entry_paths)
			{
				ImGui::TableNextColumn();
				DrawPathEntry(path, false, entry_table_state, fio_queue);
			}
		}

		if (ImGui::IsWindowHovered())
		{
			if (imtk::nav::lmb().clicked && !imtk::nav::shift().down && !imtk::nav::ctrl().down)
				ClearSelection();
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

	void ContentBrowserPanel::DrawPathEntry(const std::filesystem::path& path, bool dotdot, const EntryTableState& entry_table_state, CompoundUndoActionQueue& fio_queue)
	{
		imtk::id_scope id(path.string().c_str());
		if (auto _ = imtk::child(path.generic_string().c_str(), entry_table_state.entry_size, ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollbar))
		{
			imtk::popup rename_popup("Rename path", imtk::popup_config{ .center_window = imtk::center_window::appearing });

			if (!dotdot)
			{
				if (auto _ = imtk::context_menu::window())
				{
					if (ImGui::MenuItem("Open"))
						OpenPath(path);

					if (IsOnlySelected(path))
					{
						if (ImGui::MenuItem("Rename", "F2"))
							rename_popup.open();
					}

					if (ImGui::MenuItem("Delete"))
						DeletePath(path, fio_queue);

					ImGui::Separator();

					if (!detail::ResourcePath(path).get_import_path().exists() && Toolbar::IconMenuItem("Import", IconResource::Import))
					{
						ImportFromPath(path, fio_queue);
						ImGui::CloseCurrentPopup();
					}

					if (std::filesystem::is_directory(path) && Toolbar::IconMenuItem("Prune", IconResource::Prune))
					{
						PruneFromPath(path, fio_queue);
						ImGui::CloseCurrentPopup();
					}

					ImGui::Separator();

					if (Toolbar::IconMenuItem("Reveal in explorer", IconResource::FolderOpen))
					{
						PathInfo::RevealInExplorer(path, false);
						ImGui::CloseCurrentPopup();
					}
				}
			}

			std::string label = dotdot ? ".." : path.filename().generic_string();
			
			const ImVec2 padding_offset = ImGui::GetStyle().WindowPadding;
			const ImVec2 cursor = ImGui::GetCursorScreenPos();
			const ImVec2 child_size = ImGui::GetContentRegionAvail();
			
			const ImVec2 label_size = FitPathLabel(label, child_size.x);
			const ImVec2 label_offset = (child_size - label_size) * ImVec2(0.5f, 1.f);
			
			const ImVec2 icon_size = child_size - ImVec2(label_size.y, label_size.y);
			const ImVec2 icon_start = cursor + ImVec2(0.5f * (child_size.x - icon_size.x), 0.f);

			if (IsSelected(path))
			{
				ImGui::GetWindowDrawList()->AddRectFilled(cursor - padding_offset, cursor + child_size + padding_offset,
					ImGui::GetColorU32(ImGuiCol_FrameBgActive));

				if (path == _active_selected_path)
				{
					ImGui::GetWindowDrawList()->AddRect(cursor - padding_offset, cursor + child_size + padding_offset,
						ImGui::GetColorU32(ImGuiCol_TabSelectedOverline), 16.f, 0, 12.f);
				}
			}

			ImGui::GetWindowDrawList()->AddText(cursor + label_offset, Color::White, label.c_str());

			detail::ResourcePath res = path;

			if (ImGui::IsWindowHovered())
			{
				ImGui::SetTooltip(res.get_resource_shorthand().c_str());

				if (imtk::nav::lmb().clicked)
					ClickSelect(path);

				ImGui::GetWindowDrawList()->AddRectFilled(cursor - padding_offset, cursor + child_size + padding_offset, ImGui::GetColorU32(ImGuiCol_FrameBgHovered));

				if (imtk::nav::lmb().double_clicked)
					OpenPath(path);
			}

			ImGui::GetWindowDrawList()->AddImage(PathInfo::GetIcon(std::filesystem::is_directory(path) ? path : res.get_import_path().get_absolute()).ID(),
				icon_start, icon_start + icon_size);

			if (entry_table_state.focused && IsSelected(path) && !dotdot)
			{
				if (entry_table_state.enter_consumed)
					OpenPath(path);

				if (IsOnlySelected(path))
				{
					if (ImGui::Shortcut(ImGuiKey_F2, ImGuiInputFlags_RouteGlobal))
						rename_popup.open();
				}

				if (entry_table_state.delete_consumed)
					DeletePath(path, fio_queue); // TODO v9.4 confirmation popup

				// TODO v9.4 FIO operations: ctrl+c, ctrl+x, ctrl+v, etc.
			}

			ImGui::InvisibleButton("##DragDropItem", entry_table_state.entry_size);

			if (auto _ = imtk::drag_drop_source())
			{
				imtk::send_drag_drop_payload(ContentBrowserPathDDP(path.string()));
				ImGui::TextUnformatted("Drag path");
			}

			if (rename_popup.is_opening())
				_rename_buffer = path.filename().generic_string();

			if (auto d = rename_popup.draw())
			{
				if (ImGui::IsWindowAppearing())
					ImGui::SetKeyboardFocusHere();

				imtk::controls::input_text("Name", _rename_buffer);

				if (imtk::nav::escape().pressed)
					d.close();

				if (imtk::nav::enter().pressed)
				{
					d.close();

					auto action = std::make_unique<fio::RenamePathAction>();
					action->old_path = path;
					action->new_path = path.parent_path() / _rename_buffer;
					if (action->old_path != action->new_path)
						fio_queue.Append(std::move(action), true);
				}
			}
		}
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
			Editor::OpenFile(path);
	}

	void ContentBrowserPanel::NewAssetMenu()
	{
		if (Toolbar::IconMenuItem("Tileset", PathInfo::GetAssetIcon(detail::Key::Meta_Tileset)))
		{
			_new_asset = NewAssetInfo(detail::Key::Meta_Tileset, "New Tileset", "New tileset");
			ImGui::CloseCurrentPopup();
		}

		if (Toolbar::IconMenuItem("Signal", PathInfo::GetAssetIcon(detail::Key::Meta_Signal)))
		{
			_new_asset = NewAssetInfo(detail::Key::Meta_Signal, "New Signal", "New signal");
			ImGui::CloseCurrentPopup();
		}

		if (auto _ = imtk::menu("Fonts"))
		{
			if (Toolbar::IconMenuItem("Font family", PathInfo::GetAssetIcon(detail::Key::Meta_FontFamily)))
			{
				_new_asset = NewAssetInfo(detail::Key::Meta_FontFamily, "New Font Family", "New font family");
				ImGui::CloseCurrentPopup();
			}

			if (Toolbar::IconMenuItem("Raster font", PathInfo::GetAssetIcon(detail::Key::Meta_RasterFont)))
			{
				_new_asset = NewAssetInfo(detail::Key::Meta_RasterFont, "New Raster Font", "New raster font");
				ImGui::CloseCurrentPopup();
			}
		}
	}

	void ContentBrowserPanel::NewFolderMenu()
	{
		if (Toolbar::IconMenuItem("New folder", PathInfo::GetAssetIcon(detail::Key::Meta_Folder)))
		{
			_new_asset = NewAssetInfo(detail::Key::Meta_Folder, "New Folder", "New folder");
			ImGui::CloseCurrentPopup();
		}
	}

	void ContentBrowserPanel::DrawNewAssetPopups(CompoundUndoActionQueue& fio_queue)
	{
		if (!_new_asset)
			return;

		if (auto d = _new_asset->popup.draw())
		{
			if (ImGui::IsWindowAppearing())
				ImGui::SetKeyboardFocusHere();

			imtk::controls::input_text("Name", _new_asset->name);

			if (ImGui::Button("Create"))
			{
				d.close();
				CreateNewAsset(fio_queue);
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel"))
			{
				d.close();
				_new_asset.reset();
			}

			if (imtk::nav::enter().pressed)
			{
				d.close();
				CreateNewAsset(fio_queue);
			}

			if (imtk::nav::escape().pressed)
			{
				d.close();
				_new_asset.reset();
			}
		}
		else
			_new_asset.reset();
	}
	
	void ContentBrowserPanel::CreateNewAsset(CompoundUndoActionQueue& fio_queue)
	{
		std::string ext = _new_asset->type != detail::Key::Meta_Folder ? ".oly" : "";

		std::filesystem::path asset_path = _folder / (_new_asset->name + ext);

		if (std::filesystem::exists(asset_path))
		{
			size_t counter = 1;
			while (std::filesystem::exists(asset_path))
			{
				std::stringstream ss;
				ss << _new_asset->name << " (" << counter++ << ")" << ext;
				asset_path = _folder / ss.str();
			}
		}

		if (Editor::InitNewAsset(asset_path, _new_asset->type))
		{
			auto action = std::make_unique<fio::CreateAssetAction>();
			action->asset_path = asset_path;
			fio_queue.Append(std::move(action), false);
			_selected_paths = { asset_path };
			_active_selected_path = asset_path;
		}

		_new_asset.reset();
	}

	void ContentBrowserPanel::ClearSelection()
	{
		_selected_paths.clear();
		_active_selected_path.reset();
	}

	void ContentBrowserPanel::PruneSelection()
	{
		std::vector<std::filesystem::path> folders;
		std::vector<std::filesystem::path> files;
		_selectable_entry_paths.clear();
		std::error_code ec;
		for (const auto& entry : std::filesystem::directory_iterator(_folder, std::filesystem::directory_options::skip_permission_denied, ec))
		{
			const auto& path = entry.path();
			if (!PathInfo::IsImportFile(path))
			{
				if (std::filesystem::is_directory(path))
					folders.push_back(path);
				else if (std::filesystem::is_regular_file(path))
					files.push_back(path);
			}
		}

		for (auto& folder : folders)
			_selectable_entry_paths.push_back(std::move(folder));
		
		for (auto& file : files)
			_selectable_entry_paths.push_back(std::move(file));

		for (auto it = _selected_paths.begin(); it != _selected_paths.end(); )
		{
			if (std::find(_selectable_entry_paths.begin(), _selectable_entry_paths.end(), *it) == _selectable_entry_paths.end())
				it = _selected_paths.erase(it);
			else
				++it;
		}

		if (_active_selected_path && std::find(_selectable_entry_paths.begin(), _selectable_entry_paths.end(), *_active_selected_path) == _selectable_entry_paths.end())
			_active_selected_path.reset();

		if (!_active_selected_path && !_selected_paths.empty())
			_active_selected_path = _selected_paths.back();
	}

	void ContentBrowserPanel::ClickSelect(const std::filesystem::path& path)
	{
		if (imtk::nav::shift().down && _active_selected_path)
		{
			const auto active_it = std::find(_selectable_entry_paths.begin(), _selectable_entry_paths.end(), *_active_selected_path);
			const auto current_it = std::find(_selectable_entry_paths.begin(), _selectable_entry_paths.end(), path);
			const auto min_it = std::min(active_it, current_it);
			const auto max_it = std::max(active_it, current_it);

			for (auto it = min_it; it != std::next(max_it); ++it)
			{
				if (!IsSelected(*it))
					_selected_paths.push_back(*it);
			}

			_active_selected_path = path;
		}
		else if (imtk::nav::ctrl().down)
		{
			for (auto it = _selected_paths.begin(); it != _selected_paths.end(); ++it)
			{
				if (*it == path)
				{
					_selected_paths.erase(it);
					if (_active_selected_path == path)
					{
						if (_selected_paths.empty())
							_active_selected_path.reset();
						else
							_active_selected_path = _selected_paths.back();
					}

					return;
				}
			}

			_selected_paths.push_back(path);
			_active_selected_path = path;
		}
		else
		{
			_selected_paths = { path };
			_active_selected_path = path;
		}
	}
	
	bool ContentBrowserPanel::IsSelected(const std::filesystem::path& path) const
	{
		for (auto it = _selected_paths.begin(); it != _selected_paths.end(); ++it)
		{
			if (*it == path)
				return true;
		}

		return false;
	}

	bool ContentBrowserPanel::IsOnlySelected(const std::filesystem::path& path) const
	{
		return _selected_paths.size() == 1 && _selected_paths[0] == path;
	}

	void ContentBrowserPanel::DeletePath(const std::filesystem::path& path, CompoundUndoActionQueue& fio_queue) const
	{
		auto action = std::make_unique<fio::DeletePathAction>();
		action->del_path = path;
		fio_queue.Append(std::move(action), true);
	}

	void ContentBrowserPanel::ImportFromPath(const std::filesystem::path& path, CompoundUndoActionQueue& fio_queue)
	{
		if (std::filesystem::is_regular_file(path))
		{
			ImportFile(path, fio_queue);
			return;
		}

		_import_folder = ImportFolderInfo(path);
	}
	
	void ContentBrowserPanel::ImportFile(const std::filesystem::path& path, CompoundUndoActionQueue& fio_queue)
	{
		detail::ResourcePath source_path = path;
		if (Editor::ImportAsset(source_path))
		{
			auto action = std::make_unique<fio::CreateAssetAction>();
			action->asset_path = source_path.get_import_path();
			fio_queue.Append(std::move(action), false);
		}
	}

	void ContentBrowserPanel::DrawImportFolderPopup(CompoundUndoActionQueue& fio_queue)
	{
		if (!_import_folder)
			return;

		if (auto d = _import_folder->popup.draw())
		{
			ImGui::TextUnformatted("Folder");
			ImGui::SameLine();
			std::string folder = detail::ResourcePath(_import_folder->folder).get_resource_shorthand();
			ImGui::InputText("##Folder", folder.data(), folder.size() + 1, ImGuiInputTextFlags_ReadOnly);

			if (ImGui::Button("Import direct contents only"))
			{
				d.close();

				std::error_code ec;
				for (const auto& entry : std::filesystem::directory_iterator(detail::ResourcePath(folder).get_absolute(), std::filesystem::directory_options::skip_permission_denied, ec))
				{
					const auto& path = entry.path();
					if (std::filesystem::is_regular_file(path))
						ImportFile(path, fio_queue);
				}

				_import_folder.reset();
			}

			if (ImGui::Button("Import recursively"))
			{
				d.close();

				std::error_code ec;
				std::vector<std::filesystem::path> folders;
				folders.push_back(detail::ResourcePath(folder).get_absolute());

				while (!folders.empty())
				{
					std::filesystem::path folder = folders.back();
					folders.pop_back();

					for (const auto& entry : std::filesystem::directory_iterator(folder, std::filesystem::directory_options::skip_permission_denied, ec))
					{
						const auto& path = entry.path();
						if (std::filesystem::is_regular_file(path))
							ImportFile(path, fio_queue);
					}
				}

				_import_folder.reset();
			}

			if (imtk::nav::escape().pressed)
			{
				d.close();
				_import_folder.reset();
			}
		}
		else
			_import_folder.reset();
	}

	void ContentBrowserPanel::PruneFromPath(const std::filesystem::path& path, CompoundUndoActionQueue& fio_queue)
	{
		_prune_folder = PruneFolderInfo(path);
	}

	void ContentBrowserPanel::PrunePath(const std::filesystem::path& path, CompoundUndoActionQueue& fio_queue)
	{
		if (!PathInfo::IsImportFile(path))
			return;

		detail::ResourcePath import_path = path;
		detail::ResourcePath source_asset = import_path.get_source_path();
		if (!source_asset.exists())
		{
			auto action = std::make_unique<fio::DeletePathAction>();
			action->del_path = import_path;
			fio_queue.Append(std::move(action), true);
		}
	}

	void ContentBrowserPanel::DrawPruneFolderPopup(CompoundUndoActionQueue& fio_queue)
	{
		if (!_prune_folder)
			return;

		if (auto d = _prune_folder->popup.draw())
		{
			ImGui::TextUnformatted("Folder");
			ImGui::SameLine();
			std::string folder = detail::ResourcePath(_prune_folder->folder).get_resource_shorthand();
			ImGui::InputText("##Folder", folder.data(), folder.size() + 1, ImGuiInputTextFlags_ReadOnly);

			if (ImGui::Button("Prune direct contents only"))
			{
				d.close();

				std::error_code ec;
				for (const auto& entry : std::filesystem::directory_iterator(detail::ResourcePath(folder).get_absolute(), std::filesystem::directory_options::skip_permission_denied, ec))
					PrunePath(entry.path(), fio_queue);

				_prune_folder.reset();
			}

			if (ImGui::Button("Prune recursively"))
			{
				d.close();

				std::error_code ec;
				std::vector<std::filesystem::path> folders;
				folders.push_back(detail::ResourcePath(folder).get_absolute());

				while (!folders.empty())
				{
					std::filesystem::path folder = folders.back();
					folders.pop_back();

					for (const auto& entry : std::filesystem::directory_iterator(folder, std::filesystem::directory_options::skip_permission_denied, ec))
						PrunePath(entry.path(), fio_queue);
				}

				_prune_folder.reset();
			}

			if (imtk::nav::escape().pressed)
			{
				d.close();
				_prune_folder.reset();
			}
		}
		else
			_prune_folder.reset();
	}

	ContentBrowserPathDDP::ContentBrowserPathDDP(std::string path)
		: path(std::move(path))
	{
	}

	void ContentBrowserPathDDP::send(const std::function<void(const void*, size_t)>& dump) const
	{
		dump(path.data(), path.size());
	}
}

std::string_view imtk::drag_drop_convert<oly::editor::ContentBrowserPathDDP>::view(const void* buf, size_t size) const
{
	return std::string_view(static_cast<const char*>(buf), size);
}
