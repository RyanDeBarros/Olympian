#include "PreferencesDocument.h"

#include "core/editor/Logger.h"
#include "core/editor/ProjectInfo.h"

#include "gui/scopes/Form.h"
#include "gui/scopes/Subform.h"

namespace oly::editor
{
	const char* PreferencesDocument::GetVersion()
	{
		return "1.0";
	}

	PreferencesDocument::PreferencesDocument()
		: IDocument(ProjectInfo::Instance().EditorRoot() / "preferences.toml")
	{
	}

	void PreferencesDocument::InitImpl()
	{
		Load();
	}

	void PreferencesDocument::Draw()
	{
		gui::PropertyGrid grid;

		Draw(_scratch);

		if (gui::PropertyGrid::DirtyGrid())
			MarkDirty();
	}

	void PreferencesDocument::DrawMenuBar()
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Apply Changes"))
					ApplyChanges();

				if (ImGui::MenuItem("Save Changes", "Ctrl+S"))
					Dump();

				if (ImGui::MenuItem("Discard Changes"))
					Load();

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}
	}

	void PreferencesDocument::Load()
	{
		std::filesystem::path path = GetOlyPath().get_absolute();
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
		_in_effect = _disk;
		ActiveDescChanged();
		MarkClean();
	}

	void PreferencesDocument::Dump()
	{
		toml::table table;
		Dump(table, _scratch);
		std::filesystem::path path = GetOlyPath().get_absolute();
		std::filesystem::create_directories(path.parent_path());
		std::ofstream file(path);
		file << table;
		_disk = _scratch;
		_in_effect = _disk;
		ActiveDescChanged();
		MarkClean();
	}

	void PreferencesDocument::ApplyChanges()
	{
		_in_effect = _scratch;
		ActiveDescChanged();
	}

	const PreferencesDesc& PreferencesDocument::GetActiveDesc() const
	{
		return _in_effect;
	}

	void PreferencesDocument::ActiveDescChanged()
	{
		OnActiveDescChanged.invoke();
	}

	void PreferencesDocument::Draw(PreferencesDesc& desc)
	{
		if (auto form = Form())
		{
			if (auto pause = FormPause())
				ImGui::SeparatorText("Editor Preferences");

			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			if (auto subform = Subform("Edit"))
				Draw(desc.edit);
		
			if (auto subform = Subform("Tree View"))
				Draw(desc.tree_view);
		}
	}

	void PreferencesDocument::Draw(EditSettingsDesc& desc)
	{
		if (auto subform = Subform("Undo History"))
			Draw(desc.undo_history);
	}
	
	void PreferencesDocument::Draw(UndoHistorySettingsDesc& desc)
	{
		DRAW_FIELDS(UNDO_HISTORY_SETTINGS_GENERATOR);
	}

	void PreferencesDocument::Draw(TreeViewSettingsDesc& desc)
	{
		if (auto subform = Subform("Advanced##TreeView"))
			Draw(desc.advanced);
	}

	void PreferencesDocument::Draw(TreeViewAdvancedSettingsDesc& desc)
	{
		DRAW_FIELDS(TREE_VIEW_ADVANCED_SETTINGS_GENERATOR);
	}

	void PreferencesDocument::Load(TOMLNode node, PreferencesDesc& desc)
	{
		Load(node[detail::encode_key(desc.edit_key)], desc.edit);
		Load(node[detail::encode_key(desc.tree_view_key)], desc.tree_view);
	}

	void PreferencesDocument::Load(TOMLNode node, EditSettingsDesc& desc)
	{
		Load(node[detail::encode_key(desc.undo_history_key)], desc.undo_history);
	}

	void PreferencesDocument::Load(TOMLNode node, UndoHistorySettingsDesc& desc)
	{
		LOAD_FIELDS(UNDO_HISTORY_SETTINGS_GENERATOR);
	}

	void PreferencesDocument::Load(TOMLNode node, TreeViewSettingsDesc& desc)
	{
		Load(node[detail::encode_key(desc.advanced_key)], desc.advanced);
	}

	void PreferencesDocument::Load(TOMLNode node, TreeViewAdvancedSettingsDesc& desc)
	{
		LOAD_FIELDS(TREE_VIEW_ADVANCED_SETTINGS_GENERATOR);
	}

	void PreferencesDocument::Dump(toml::table& table, PreferencesDesc& desc)
	{
		toml::table subtable;

		subtable.clear();
		Dump(subtable, desc.edit);
		table.insert_or_assign(detail::encode_key(desc.edit_key), std::move(subtable));

		subtable.clear();
		Dump(subtable, desc.tree_view);
		table.insert_or_assign(detail::encode_key(desc.tree_view_key), std::move(subtable));
	}

	void PreferencesDocument::Dump(toml::table& table, EditSettingsDesc& desc)
	{
		toml::table subtable;
		Dump(subtable, desc.undo_history);
		table.insert_or_assign(detail::encode_key(desc.undo_history_key), std::move(subtable));
	}

	void PreferencesDocument::Dump(toml::table& table, UndoHistorySettingsDesc& desc)
	{
		DUMP_FIELDS(UNDO_HISTORY_SETTINGS_GENERATOR);
	}

	void PreferencesDocument::Dump(toml::table& table, TreeViewSettingsDesc& desc)
	{
		toml::table subtable;
		Dump(subtable, desc.advanced);
		table.insert_or_assign(detail::encode_key(desc.advanced_key), std::move(subtable));
	}

	void PreferencesDocument::Dump(toml::table& table, TreeViewAdvancedSettingsDesc& desc)
	{
		DUMP_FIELDS(TREE_VIEW_ADVANCED_SETTINGS_GENERATOR);
	}
}
