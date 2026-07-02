#include "ContentBrowserPanel.h"

#include "core/Errors.h"
#include "core/PathInfo.h"

#include "core/editor/Editor.h"
#include "core/editor/LiveSettings.h"
#include "core/editor/Logger.h"
#include "core/editor/ProjectInfo.h"
#include "core/editor/ResourceLoader.h"
#include "core/editor/UID.h"

#include "core/windows/MainWindow.h"

#include "panels/PanelManager.h"

#include "gui/Controls.h"
#include "gui/ImGuiWrapper.h"
#include "gui/graphics/Toolbar.h"

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
		// TODO v9.2 if GetFavoritesList() does not include root, add it at beginning
		SetFolder(ProjectInfo::Instance().ResourceRoot());
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
				int columns = *Editor::GetLiveSettings().content_browser->columns;
				ImGui::SetNextItemWidth(100.f);
				ImGui::InputInt("Columns", &columns);
				*Editor::GetLiveSettings().content_browser->columns = std::max(columns, 1);

				gui::VerticalSeparator();

				gui::FloatControl("Font scale", *Editor::GetLiveSettings().content_browser->font_scale, 120.f, 0.1f, 10.f, "%.1f", true);

				gui::VerticalSeparator();
				
				// TODO v9.2 button to show folder in tree view panel

				if (auto disabled = DisabledSection(_on_res_root))
				{
					if (Toolbar::DrawIconToggleButton(IconResource::StarFilled, IconResource::StarOutline, _favorited, "Favorite"))
					{
						if (!disabled.Disabled())
							SyncFavoritesList();
					}
				}
				// TODO v9.2 toolbar for '<'/'>' (keep stack of folder history so as to go back and forth between folders), favorites button (pop out modal with list of favorites / star button to toggle favorite status of current folder / etc.), etc.

				const float font_global_scale = ImGui::GetIO().FontGlobalScale;
				ImGui::GetIO().FontGlobalScale *= *Editor::GetLiveSettings().content_browser->font_scale;

				if (ImGui::BeginTable("##Table", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp))
				{
					ImGui::TableNextRow();

					ImGui::TableSetColumnIndex(0);
					DrawFavoritesList();

					ImGui::TableSetColumnIndex(1);
					DrawFolderView();
					
					ImGui::EndTable();
				}

				ImGui::GetIO().FontGlobalScale = font_global_scale;
			}

			ImGui::EndChild();
		}
	}

	void ContentBrowserPanel::ShowInContentBrowser(const detail::ResourcePath& path)
	{
		if (path.is_resource())
		{
			if (path.is_directory())
				SetFolder(path.get_absolute());
			else
				SetFolder(path.get_absolute().parent_path());
		}
		else
			MainWindow::Instance().PushNotification(Notification(LogLevel::Error, path.string() + " is not located in the project resource folder"));
	}

	void ContentBrowserPanel::ShowInContentBrowser(const std::filesystem::path& path)
	{
		if (detail::ResourcePath(path).is_resource())
		{
			if (std::filesystem::is_directory(path))
				SetFolder(path);
			else
				SetFolder(path.parent_path());
		}
		else
			MainWindow::Instance().PushNotification(Notification(LogLevel::Error, path.generic_string() + " is not located in the project resource folder"));
	}

	void ContentBrowserPanel::SetFolder(std::filesystem::path folder)
	{
		_folder = std::move(folder);
		_favorited = ShouldBeFavorited();
		_on_res_root = std::filesystem::equivalent(_folder, ProjectInfo::Instance().ResourceRoot());
		_selected_path.reset();
	}

	std::vector<std::string>& ContentBrowserPanel::GetFavoritesList() const
	{
		return *Editor::GetLiveSettings().content_browser->favorites;
	}

	bool ContentBrowserPanel::ShouldBeFavorited() const
	{
		const auto& favorites = GetFavoritesList();
		for (const auto& favorite : favorites)
		{
			// TODO v9.2 favorites list should be a SimpleDesc<OrderedSet<ResourcePath>> with custom comparator
			if (std::filesystem::equivalent(detail::ResourcePath(favorite).get_absolute(), _folder))
				return true;
		}

		return false;
	}


	void ContentBrowserPanel::SyncFavoritesList() const
	{
		if (_favorited)
			GetFavoritesList().push_back(detail::ResourcePath(_folder).get_resource_shorthand());
		else
		{
			auto& favorites = GetFavoritesList();;
			for (auto it = favorites.begin(); it != favorites.end(); ++it)
			{
				if (std::filesystem::equivalent(detail::ResourcePath(*it).get_absolute(), _folder))
				{
					favorites.erase(it);
					break;
				}
			}
		}
	}

	void ContentBrowserPanel::DrawFavoritesList()
	{
		std::optional<std::string> open_folder;
		for (const auto& favorite : GetFavoritesList())
		{
			// TODO v9.2 ellipses for overflow
			if (ImGui::Selectable(favorite.c_str()))
				open_folder = favorite;
		}

		if (open_folder)
			ShowInContentBrowser(detail::ResourcePath(*open_folder));
	}

	void ContentBrowserPanel::DrawFolderView()
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
				DrawPathTable();
		}

		ImGui::EndChild();
	}

	void ContentBrowserPanel::DrawPathTable()
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
				DrawPathEntry(folder.parent_path(), true, path_entry_size);
			}

			std::error_code ec;
			for (const auto& entry : std::filesystem::directory_iterator(folder, std::filesystem::directory_options::skip_permission_denied, ec))
			{
				ImGui::TableNextColumn();
				DrawPathEntry(entry.path(), false, path_entry_size);
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

	void ContentBrowserPanel::DrawPathEntry(const std::filesystem::path& path, bool dotdot, const ImVec2 size)
	{
		if (ImGui::BeginChild(path.generic_string().c_str(), size, ImGuiChildFlags_Borders))
		{
			static constexpr const char* RENAME_POPUP = "Rename path";
			bool open_rename_popup = false;

			if (ImGui::BeginPopupContextWindow())
			{
				if (ImGui::MenuItem("Open"))
					OpenPath(path);

				if (ImGui::MenuItem("Rename", "F2"))
					open_rename_popup = true;

				// TODO v9.2 context menu

				ImGui::EndPopup();
			}

			std::string label = dotdot ? ".." : path.filename().generic_string();
			
			const ImVec2 padding_offset = ImGui::GetStyle().CellPadding + ImGui::GetStyle().WindowPadding;
			const ImVec2 cursor = ImGui::GetCursorScreenPos();
			const ImVec2 child_size = ImGui::GetContentRegionAvail();
			
			const ImVec2 label_size = FitPathLabel(label, child_size);
			const ImVec2 label_offset = (child_size - label_size) * ImVec2(0.5f, 1.f);
			
			const ImVec2 icon_size = child_size - ImVec2(label_size.y, label_size.y);
			const ImVec2 icon_start = cursor + ImVec2(0.5f * (child_size.x - icon_size.x), 0.f);

			if (_selected_path == path)
			{
				ImGui::GetWindowDrawList()->AddRectFilled(cursor - padding_offset, cursor + child_size + 2 * padding_offset, ImGui::GetColorU32(ImGuiCol_FrameBgActive));
				ImGui::GetWindowDrawList()->AddRect(cursor - padding_offset, cursor + child_size + 2 * padding_offset, ImGui::GetColorU32(ImGuiCol_TabSelectedOverline), 0.f, 0, 3.f);
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

				// TODO v9.2 FIO operations: ctrl+c, ctrl+x, ctrl+v, etc.
			}

			if (open_rename_popup)
				ImGui::OpenPopup(RENAME_POPUP);

			ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			if (ImGui::BeginPopupModal(RENAME_POPUP, 0, ImGuiWindowFlags_AlwaysAutoResize))
			{
				std::string filename = path.filename().generic_string();
				if (gui::InputText("Filename", filename))
				{
					// TODO v9.2 publish FIO operation to rename file -> defer until after DrawFolderView() loop. FIO operations should support undo/redo stack that's local to content browser panel.
				}

				if (ImGui::IsItemDeactivatedAfterEdit())
					ImGui::CloseCurrentPopup();

				if (ImGui::Shortcut(ImGuiKey_Escape) || ImGui::Shortcut(ImGuiKey_Enter))
					ImGui::CloseCurrentPopup();

				ImGui::EndPopup();
			}
		}

		ImGui::EndChild();
	}

	ImVec2 ContentBrowserPanel::FitPathLabel(std::string& label, const ImVec2 child_size)
	{
		ImVec2 label_size = ImGui::CalcTextSize(label.c_str());
		if (child_size.x > 0.f && label_size.x > child_size.x)
		{
			static constexpr const char* ellipses = "...";
			const ImVec2 ellipses_size = ImGui::CalcTextSize(ellipses);

			while (label_size.x + ellipses_size.x > child_size.x)
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
}
