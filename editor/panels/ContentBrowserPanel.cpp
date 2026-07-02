#include "ContentBrowserPanel.h"

#include "core/Errors.h"
#include "core/PathInfo.h"

#include "core/editor/Editor.h"
#include "core/editor/LiveSettings.h"
#include "core/editor/Logger.h"
#include "core/editor/ProjectInfo.h"

#include "core/windows/MainWindow.h"

#include "panels/PanelManager.h"

#include "gui/ImGuiWrapper.h"
#include "gui/Controls.h"

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
		_selected_path.reset();
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

				const float font_global_scale = ImGui::GetIO().FontGlobalScale;
				ImGui::GetIO().FontGlobalScale *= *Editor::GetLiveSettings().content_browser->font_scale;

				// TODO v9.2 toolbar for '<'/'>' (keep stack of folder history so as to go back and forth between folders), favorites button (pop out modal with list of favorites / star button to toggle favorite status of current folder / etc.), etc.

				DrawFolderView();

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
				_folder = path.get_absolute();
			else
				_folder = path.get_absolute().parent_path();

			_selected_path.reset();
		}
		else
			MainWindow::Instance().PushNotification(Notification(LogLevel::Error, path.string() + " is not located in the project resource folder"));
	}

	void ContentBrowserPanel::ShowInContentBrowser(const std::filesystem::path& path)
	{
		if (detail::ResourcePath(path).is_resource())
		{
			if (std::filesystem::is_directory(path))
				_folder = path;
			else
				_folder = path.parent_path();

			_selected_path.reset();
		}
		else
			MainWindow::Instance().PushNotification(Notification(LogLevel::Error, path.generic_string() + " is not located in the project resource folder"));
	}

	void ContentBrowserPanel::DrawFolderView()
	{
		if (ImGui::BeginChild("##FolderView", ImVec2(0, 0), ImGuiChildFlags_Borders))
		{
			// TODO v9.2 context menu

			const unsigned int columns = *Editor::GetLiveSettings().content_browser->columns;
			if (ImGui::BeginTable("##PathEntryTable", columns, ImGuiTableFlags_SizingFixedSame))
			{
				const float full_width = ImGui::GetContentRegionAvail().x - columns * 2 * ImGui::GetStyle().CellPadding.x;
				const float width = full_width / columns;
				const ImVec2 path_entry_size(width, width);

				ImGui::TableNextRow();

				const std::filesystem::path folder = _folder;

				if (!std::filesystem::equivalent(folder, ProjectInfo::Instance().ResourceRoot()))
				{
					ImGui::TableNextColumn();
					DrawPathEntry(folder.parent_path(), "..", path_entry_size);
				}

				std::error_code ec;
				for (const auto& entry : std::filesystem::directory_iterator(folder, std::filesystem::directory_options::skip_permission_denied, ec))
				{
					ImGui::TableNextColumn();
					DrawPathEntry(entry.path(), nullptr, path_entry_size);
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

		ImGui::EndChild();
	}

	void ContentBrowserPanel::DrawPathEntry(const std::filesystem::path& path, const char* label_override, const ImVec2 size)
	{
		if (ImGui::BeginChild(path.generic_string().c_str(), size, ImGuiChildFlags_Borders))
		{
			// TODO v9.2 context menu

			std::string label = label_override ? label_override : path.filename().generic_string();
			
			const ImVec2 padding_offset = ImGui::GetStyle().CellPadding + ImGui::GetStyle().WindowPadding;
			const ImVec2 cursor = ImGui::GetCursorScreenPos();
			const ImVec2 child_size = ImGui::GetContentRegionAvail();
			
			// TODO v9.1 UI control next to column count for path label font size
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

			if (ImGui::IsWindowFocused() && _selected_path == path)
			{
				if (ImGui::Shortcut(ImGuiKey_Enter, ImGuiInputFlags_RouteGlobal))
					OpenPath(path);

				if (ImGui::Shortcut(ImGuiKey_F2, ImGuiInputFlags_RouteGlobal))
				{
					// TODO v9.2 rename popup
				}

				// TODO v9.2 ctrl+c, ctrl+x, ctrl+v, etc.
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
