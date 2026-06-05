#include "PreferencesPanel.h"

#include "core/MainWindow.h"
#include "core/Errors.h"
#include "core/ProjectInfo.h"
#include "panels/PanelManager.h"

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
		auto window = DrawDockedWindow(IsDirty() ? ImGuiWindowFlags_UnsavedDocument : ImGuiWindowFlags_None);
		if (window.RequestsClose() && IsDirty())
		{
			Open();
			ImGui::SetWindowFocus();
		}

		if (window.IsVisible())
		{
			if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_S, ImGuiInputFlags_RouteGlobal))
				Dump();

			Draw(_scratch);
		}
	}

	std::filesystem::path PreferencesPanel::GetPath() const
	{
		return ProjectInfo::Instance().EditorRoot() / "preferences.toml";
	}

	void PreferencesPanel::Load()
	{
		// TODO v8
	}

	void PreferencesPanel::Dump()
	{
		// TODO v8
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
		if (DescIO::BeginForm(this))
		{
			DescIO::FormSeparator(this, "Editor Preferences");
			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			Draw(desc.tree_view);

			DescIO::EndForm();
		}
	}

	void PreferencesPanel::Draw(TreeViewSettingsDesc& desc)
	{
		if (auto section = DescIO::CollapsingSection(this, "Advanced##TreeView"))
			Draw(desc.advanced);
	}

	void PreferencesPanel::Draw(TreeViewAdvancedSettingsDesc& desc)
	{
		DRAW_FIELDS(TREE_NODE_ADVANCED_SETTINGS_GENERATOR);
	}

	void PreferencesPanel::Load(TOMLNode node, PreferencesDesc& desc)
	{
		Load(node[detail::encode_key(detail::Key::TreeView)], desc.tree_view);
	}
	
	void PreferencesPanel::Load(TOMLNode node, TreeViewSettingsDesc& desc)
	{
		Load(node[detail::encode_key(detail::Key::Advanced)], desc.advanced);
	}
	
	void PreferencesPanel::Load(TOMLNode node, TreeViewAdvancedSettingsDesc& desc)
	{
		LOAD_FIELDS(TREE_NODE_ADVANCED_SETTINGS_GENERATOR);
	}

	void PreferencesPanel::Dump(toml::table& table, PreferencesDesc& desc)
	{
		toml::table subtable;
		Dump(subtable, desc.tree_view);
		table.insert_or_assign(detail::encode_key(detail::Key::TreeView), std::move(subtable));
	}
	
	void PreferencesPanel::Dump(toml::table& table, TreeViewSettingsDesc& desc)
	{
		toml::table subtable;
		Dump(table, desc.advanced);
		table.insert_or_assign(detail::encode_key(detail::Key::Advanced), std::move(subtable));
	}
	
	void PreferencesPanel::Dump(toml::table& table, TreeViewAdvancedSettingsDesc& desc)
	{
		DUMP_FIELDS(TREE_NODE_ADVANCED_SETTINGS_GENERATOR);
	}
}
