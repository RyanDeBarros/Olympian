#include "PreferencesPanel.h"

#include "core/windows/MainWindow.h"
#include "core/Errors.h"
#include "core/editor/Logger.h"
#include "core/editor/ProjectInfo.h"
#include "panels/PanelManager.h"

#include "gui/scopes/Form.h"
#include "gui/scopes/Subform.h"

#include "definitions/Keys.h"

#include <imgui.h>

namespace oly::editor
{
	PreferencesPanel& PreferencesPanel::Instance()
	{
		if (auto panel = MainWindow::Instance().GetPanelManager().Get<PreferencesPanel>())
			return *panel;
		else
			BreakoutError::Throw("No instance of PreferencesPanel");
	}

	void PreferencesPanel::Init()
	{
		Load();
	}

	const char* PreferencesPanel::GetTitle() const
	{
		return "Preferences";
	}

	void PreferencesPanel::Draw()
	{
		ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar;
		if (IsDirty())
			flags |= ImGuiWindowFlags_UnsavedDocument;

		auto window = DrawDockedWindow(flags);
		if (window.RequestsClose() && IsDirty())
		{
			Open();
			ImGui::SetWindowFocus();
		}

		if (window.IsVisible())
		{
			if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_S, ImGuiInputFlags_RouteGlobal))
				Dump();

			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::MenuItem("Save Changes", "Ctrl+S"))
						Dump();

					if (ImGui::MenuItem("Discard Changes"))
						Load();

					ImGui::EndMenu();
				}

				ImGui::EndMenuBar();
			}

			Draw(_scratch);
		}
	}

	std::filesystem::path PreferencesPanel::GetPath() const
	{
		return ProjectInfo::Instance().EditorRoot() / "preferences.toml";
	}

	void PreferencesPanel::Load()
	{
		std::filesystem::path path = GetPath();
		toml::table table;
		if (std::filesystem::is_regular_file(path))
		{
			try
			{
				table = toml::parse_file(path.string());
			}
			catch (const toml::parse_error& e)
			{
				Logger::Instance().Log(LogLevel::Warning, "Cannot load editor preferences: " + std::string(e.what()));
			}
		}

		Load(TOMLNode(table), _disk);
		_scratch = _disk;
		MarkClean();
	}

	void PreferencesPanel::Dump()
	{
		toml::table table;
		Dump(table, _scratch);
		std::filesystem::path path = GetPath();
		std::filesystem::create_directories(path.parent_path());
		std::ofstream file(path);
		file << table;
		_disk = _scratch;
		MarkClean();
	}

	void PreferencesPanel::MarkDirty()
	{
		_dirty = true;
	}

	void PreferencesPanel::MarkClean()
	{
		_dirty = false;
	}

	bool PreferencesPanel::IsDirty() const
	{
		return _dirty;
	}

	const PreferencesDesc& PreferencesPanel::GetSavedDesc() const
	{
		return _disk;
	}

	void PreferencesPanel::Draw(PreferencesDesc& desc)
	{
		if (auto form = Form())
		{
			if (auto pause = FormPause())
				ImGui::SeparatorText("Editor Preferences");

			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			Draw(desc.tree_view);
		}
	}

	void PreferencesPanel::Draw(TreeViewSettingsDesc& desc)
	{
		if (auto subform = Subform("Advanced##TreeView"))
			Draw(desc.advanced);
	}

	void PreferencesPanel::Draw(TreeViewAdvancedSettingsDesc& desc)
	{
		DRAW_FIELDS(TREE_VIEW_ADVANCED_SETTINGS_GENERATOR);
	}

	void PreferencesPanel::Load(TOMLNode node, PreferencesDesc& desc)
	{
		Load(node[detail::encode_key(desc.tree_view_key)], desc.tree_view);
	}
	
	void PreferencesPanel::Load(TOMLNode node, TreeViewSettingsDesc& desc)
	{
		Load(node[detail::encode_key(desc.advanced_key)], desc.advanced);
	}
	
	void PreferencesPanel::Load(TOMLNode node, TreeViewAdvancedSettingsDesc& desc)
	{
		LOAD_FIELDS(TREE_VIEW_ADVANCED_SETTINGS_GENERATOR);
	}

	void PreferencesPanel::Dump(toml::table& table, PreferencesDesc& desc)
	{
		toml::table subtable;
		Dump(subtable, desc.tree_view);
		table.insert_or_assign(detail::encode_key(desc.tree_view_key), std::move(subtable));
	}
	
	void PreferencesPanel::Dump(toml::table& table, TreeViewSettingsDesc& desc)
	{
		toml::table subtable;
		Dump(subtable, desc.advanced);
		table.insert_or_assign(detail::encode_key(desc.advanced_key), std::move(subtable));
	}
	
	void PreferencesPanel::Dump(toml::table& table, TreeViewAdvancedSettingsDesc& desc)
	{
		DUMP_FIELDS(TREE_VIEW_ADVANCED_SETTINGS_GENERATOR);
	}
}
