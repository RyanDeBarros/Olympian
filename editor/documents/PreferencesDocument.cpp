#include "PreferencesDocument.h"

#include "core/editor/Editor.h"
#include "core/editor/ProjectInfo.h"

#include "fio/Trashcan.h"

#include "assets/TranslateKey.h"

// TODO v9.4 Defaults for descriptors should come from preferences sub-descriptors -> Create an "Asset Defaults" subform.

namespace oly::editor
{
	const char* PreferencesDocument::GetVersion()
	{
		return "1.0";
	}

	PreferencesDocument::PreferencesDocument()
		: IDocument(ProjectInfo::Instance().EditorRoot() / "Preferences.toml")
	{
	}

	void PreferencesDocument::InitImpl()
	{
		LoadAsset();
	}

	void PreferencesDocument::Draw()
	{
		auto pre_draw = PreDraw();

		Draw(_desc.scratch);

		if (pre_draw.grid.dirty())
			MarkDirty();
	}

	void PreferencesDocument::DrawMenuBar()
	{
		if (auto _ = imtk::menu_bar())
		{
			if (auto _ = imtk::menu("File"))
			{
				if (ImGui::MenuItem("Apply Changes"))
					ApplyEditorPreferences();

				if (ImGui::MenuItem("Save Changes", "Ctrl+S"))
					DumpAsset();

				if (ImGui::MenuItem("Discard Changes"))
					LoadAsset();

				if (ImGui::MenuItem("Reset Preferences"))
					ResetAsset();
			}
		}
	}

	void PreferencesDocument::LoadImpl()
	{
		std::filesystem::path path = _oly_path.get_absolute();
		toml::table table;
		if (std::filesystem::is_regular_file(path))
		{
			try
			{
				table = toml::parse_file(path.string());
			}
			catch (const toml::parse_error& e)
			{
				imtk::log_warning("Cannot load editor preferences: " + std::string(e.what()));
			}
		}

		Load(imtk::toml_node(table), _desc.disk);

		_desc.LoadFromDisk();
		RevertEditorPreferences();
		MarkClean();
	}

	void PreferencesDocument::DumpImpl()
	{
		toml::table table;
		Dump(table, _desc.scratch);
		std::filesystem::path path = _oly_path.get_absolute();
		std::filesystem::create_directories(path.parent_path());
		std::ofstream file(path);
		file << table;
		_desc.WriteToDisk();
		RevertEditorPreferences();
		MarkClean();
	}

	void PreferencesDocument::ResetAssetImpl()
	{
		Load(imtk::toml_node(), _desc.scratch);
	}

	const IDoubleDescriptor& PreferencesDocument::GetDoubleDescriptor() const
	{
		return _desc;
	}

	IDoubleDescriptor& PreferencesDocument::GetDoubleDescriptor()
	{
		return _desc;
	}

	void PreferencesDocument::ApplyEditorPreferences()
	{
		Editor::GetPreferences().copy_data(_desc.scratch);
		ActiveDescChanged();
	}

	void PreferencesDocument::RevertEditorPreferences()
	{
		Editor::GetPreferences().copy_data(_desc.disk);
		ActiveDescChanged();
	}

	void PreferencesDocument::ActiveDescChanged()
	{
		Editor::OnPreferencesChanged().invoke();
	}

	void PreferencesDocument::Draw(PreferencesDesc& desc)
	{
		if (auto form = imtk::prop::form())
		{
			if (auto pause = imtk::prop::form::pause())
				ImGui::SeparatorText("Editor Preferences");

			if (imtk::prop::in_form())
			{
				if (auto subform = imtk::prop::subform("Edit"))
					Draw(*desc.edit);

				if (auto subform = imtk::prop::subform("Content Browser"))
					Draw(*desc.content_browser);

				if (auto subform = imtk::prop::subform("Tree View"))
					Draw(*desc.tree_view);

				if (auto subform = imtk::prop::subform("Filesystem"))
					Draw(*desc.filesystem);
			}
		}
	}

	void PreferencesDocument::Draw(EditSettingsDesc& desc)
	{
		if (auto subform = imtk::prop::subform("Undo History"))
			Draw(*desc.undo_history);
	}
	
	void PreferencesDocument::Draw(UndoHistorySettingsDesc& desc)
	{
		IMTK_DRAW_FIELDS(UNDO_HISTORY_SETTINGS_GENERATOR);
	}

	void PreferencesDocument::Draw(ContentBrowserSettingsDesc& desc)
	{
		IMTK_DRAW_FIELDS(CONTENT_BROWSER_SETTINGS_PARTIAL_GENERATOR);

		if (auto subform = imtk::prop::subform("Undo History"))
			Draw(*desc.undo_history);
	}

	void PreferencesDocument::Draw(TreeViewSettingsDesc& desc)
	{
		if (auto subform = imtk::prop::subform("Advanced##TreeView"))
			Draw(*desc.advanced);
	}

	void PreferencesDocument::Draw(TreeViewAdvancedSettingsDesc& desc)
	{
		IMTK_DRAW_FIELDS(TREE_VIEW_ADVANCED_SETTINGS_GENERATOR);
	}

	void PreferencesDocument::Draw(FilesystemSettingsDesc& desc)
	{
		IMTK_DRAW_FIELDS(FILESYSTEM_SETTINGS_GENERATOR);

		if (auto subform = imtk::prop::subform("Advanced"))
		{
			imtk::prop::key::set_label("Estimated trash folder size");
			imtk::prop::value::add_component(std::make_unique<imtk::w::readonly_text_owned>(std::to_string(fio::Trashcan::EstimatedSize()) + " bytes"));
			imtk::prop::row::submit();

			imtk::popup clear_trash_popup("Clear trash folder", imtk::popup_config{ .center_window = imtk::center_window::appearing, .modal = true, .window_flags = ImGuiWindowFlags_AlwaysAutoResize });

			if (auto pause = imtk::prop::form::pause())
			{
				if (ImGui::Button("Clear trash folder"))
					clear_trash_popup.open();
			}

			if (auto d = clear_trash_popup.draw())
			{
				ImGui::TextUnformatted("Are you sure? This action is irreversible.");

				if (ImGui::Button("Confirm"))
				{
					fio::Trashcan::ForceClear();
					d.close();
				}

				ImGui::SameLine();

				if (ImGui::Button("Cancel"))
					d.close();
			}
		}
	}

	void PreferencesDocument::Load(imtk::toml_node node, PreferencesDesc& desc)
	{
		Load(desc.edit.subnode(node), *desc.edit);
		Load(desc.content_browser.subnode(node), *desc.content_browser);
		Load(desc.tree_view.subnode(node), *desc.tree_view);
		Load(desc.filesystem.subnode(node), *desc.filesystem);
	}

	void PreferencesDocument::Load(imtk::toml_node node, EditSettingsDesc& desc)
	{
		Load(desc.undo_history.subnode(node), *desc.undo_history);
	}

	void PreferencesDocument::Load(imtk::toml_node node, UndoHistorySettingsDesc& desc)
	{
		IMTK_LOAD_FIELDS(UNDO_HISTORY_SETTINGS_GENERATOR);
	}

	void PreferencesDocument::Load(imtk::toml_node node, ContentBrowserSettingsDesc& desc)
	{
		IMTK_LOAD_FIELDS(CONTENT_BROWSER_SETTINGS_PARTIAL_GENERATOR);

		Load(desc.undo_history.subnode(node), *desc.undo_history);
	}

	void PreferencesDocument::Load(imtk::toml_node node, TreeViewSettingsDesc& desc)
	{
		Load(desc.advanced.subnode(node), *desc.advanced);
	}

	void PreferencesDocument::Load(imtk::toml_node node, TreeViewAdvancedSettingsDesc& desc)
	{
		IMTK_LOAD_FIELDS(TREE_VIEW_ADVANCED_SETTINGS_GENERATOR);
	}

	void PreferencesDocument::Load(imtk::toml_node node, FilesystemSettingsDesc& desc)
	{
		IMTK_LOAD_FIELDS(FILESYSTEM_SETTINGS_GENERATOR);
	}

	void PreferencesDocument::Dump(toml::table& table, PreferencesDesc& desc)
	{
		toml::table subtable;

		subtable.clear();
		Dump(subtable, *desc.edit);
		desc.edit.dump_into(table, std::move(subtable));

		subtable.clear();
		Dump(subtable, *desc.content_browser);
		desc.content_browser.dump_into(table, std::move(subtable));

		subtable.clear();
		Dump(subtable, *desc.tree_view);
		desc.tree_view.dump_into(table, std::move(subtable));

		subtable.clear();
		Dump(subtable, *desc.filesystem);
		desc.filesystem.dump_into(table, std::move(subtable));
	}

	void PreferencesDocument::Dump(toml::table& table, EditSettingsDesc& desc)
	{
		toml::table subtable;
		Dump(subtable, *desc.undo_history);
		desc.undo_history.dump_into(table, std::move(subtable));
	}

	void PreferencesDocument::Dump(toml::table& table, UndoHistorySettingsDesc& desc)
	{
		IMTK_DUMP_FIELDS(UNDO_HISTORY_SETTINGS_GENERATOR);
	}

	void PreferencesDocument::Dump(toml::table& table, ContentBrowserSettingsDesc& desc)
	{
		IMTK_DUMP_FIELDS(CONTENT_BROWSER_SETTINGS_PARTIAL_GENERATOR);

		toml::table subtable;
		Dump(subtable, *desc.undo_history);
		desc.undo_history.dump_into(table, std::move(subtable));
	}

	void PreferencesDocument::Dump(toml::table& table, TreeViewSettingsDesc& desc)
	{
		toml::table subtable;
		Dump(subtable, *desc.advanced);
		desc.advanced.dump_into(table, std::move(subtable));
	}

	void PreferencesDocument::Dump(toml::table& table, TreeViewAdvancedSettingsDesc& desc)
	{
		IMTK_DUMP_FIELDS(TREE_VIEW_ADVANCED_SETTINGS_GENERATOR);
	}

	void PreferencesDocument::Dump(toml::table& table, FilesystemSettingsDesc& desc)
	{
		IMTK_DUMP_FIELDS(FILESYSTEM_SETTINGS_GENERATOR);
	}
}
