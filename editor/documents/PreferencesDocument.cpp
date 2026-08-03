#include "PreferencesDocument.h"

#include "core/editor/Editor.h"
#include "core/editor/Logger.h"
#include "core/editor/ProjectInfo.h"

#include "gui/scopes/Form.h"
#include "gui/scopes/Subform.h"

#include "fio/Trashcan.h"

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

		Draw(DataPath(), _desc.scratch);

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
					ApplyEditorPreferences();

				if (ImGui::MenuItem("Save Changes", "Ctrl+S"))
					DumpAsset();

				if (ImGui::MenuItem("Discard Changes"))
					LoadAsset();

				if (ImGui::MenuItem("Reset Preferences"))
					ResetAsset();

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
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
				Logger::LogWarning("Cannot load editor preferences: " + std::string(e.what()));
			}
		}

		Load(TOMLNode(table), _desc.disk);

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
		Load(TOMLNode(), _desc.scratch);
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
		Editor::GetPreferences() = _desc.scratch;
		ActiveDescChanged();
	}

	void PreferencesDocument::RevertEditorPreferences()
	{
		Editor::GetPreferences() = _desc.disk;
		ActiveDescChanged();
	}

	void PreferencesDocument::ActiveDescChanged()
	{
		Editor::OnPreferencesChanged().invoke();
	}

	void PreferencesDocument::Draw(DataPath path, PreferencesDesc& desc)
	{
		if (auto form = Form())
		{
			if (auto pause = FormPause())
				ImGui::SeparatorText("Editor Preferences");

			if (Form::ValidActiveForm())
			{
				if (auto subform = Subform("Edit"))
					Draw(path / desc.subpaths.edit, desc.edit);

				if (auto subform = Subform("Content Browser"))
					Draw(path / desc.subpaths.content_browser, desc.content_browser);

				if (auto subform = Subform("Tree View"))
					Draw(path / desc.subpaths.tree_view, desc.tree_view);

				if (auto subform = Subform("Filesystem"))
					Draw(path / desc.subpaths.filesystem, desc.filesystem);
			}
		}
	}

	void PreferencesDocument::Draw(DataPath path, EditSettingsDesc& desc)
	{
		if (auto subform = Subform("Undo History"))
			Draw(path / desc.subpaths.undo_history, desc.undo_history);
	}
	
	void PreferencesDocument::Draw(DataPath path, UndoHistorySettingsDesc& desc)
	{
		DRAW_FIELDS(UNDO_HISTORY_SETTINGS_GENERATOR);
	}

	void PreferencesDocument::Draw(DataPath path, ContentBrowserSettingsDesc& desc)
	{
		DRAW_FIELDS(CONTENT_BROWSER_SETTINGS_GENERATOR);
	}

	void PreferencesDocument::Draw(DataPath path, TreeViewSettingsDesc& desc)
	{
		if (auto subform = Subform("Advanced##TreeView"))
			Draw(path / desc.subpaths.advanced, desc.advanced);
	}

	void PreferencesDocument::Draw(DataPath path, TreeViewAdvancedSettingsDesc& desc)
	{
		DRAW_FIELDS(TREE_VIEW_ADVANCED_SETTINGS_GENERATOR);
	}

	void PreferencesDocument::Draw(DataPath path, FilesystemSettingsDesc& desc)
	{
		DRAW_FIELDS(FILESYSTEM_SETTINGS_GENERATOR);

		if (auto subform = Subform("Advanced"))
		{
			gui::PropertyGrid::Key::SetLabel("Estimated trash folder size");
			gui::PropertyGrid::Value::AddComponent(comp::Generic([]() -> DrawResult {
				std::string buf = std::to_string(fio::Trashcan::EstimatedSize()) + " bytes";
				ImGui::InputText("##", buf.data(), buf.size() + 1, ImGuiInputTextFlags_ReadOnly);
				return DrawResult().Query();
			}));
			gui::PropertyGrid::SubmitRow();

			imtk::popup clear_trash_popup("Clear trash folder");

			if (auto pause = FormPause())
			{
				if (ImGui::Button("Clear trash folder"))
					clear_trash_popup.open();
			}

			ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));


			if (auto d = clear_trash_popup.draw(true, ImGuiWindowFlags_AlwaysAutoResize))
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

	void PreferencesDocument::Load(TOMLNode node, PreferencesDesc& desc)
	{
		Load(node[detail::encode_key(desc.edit_key)], desc.edit);
		Load(node[detail::encode_key(desc.content_browser_key)], desc.content_browser);
		Load(node[detail::encode_key(desc.tree_view_key)], desc.tree_view);
		Load(node[detail::encode_key(desc.filesystem_key)], desc.filesystem);
	}

	void PreferencesDocument::Load(TOMLNode node, EditSettingsDesc& desc)
	{
		Load(node[detail::encode_key(desc.undo_history_key)], desc.undo_history);
	}

	void PreferencesDocument::Load(TOMLNode node, UndoHistorySettingsDesc& desc)
	{
		LOAD_FIELDS(UNDO_HISTORY_SETTINGS_GENERATOR);
	}

	void PreferencesDocument::Load(TOMLNode node, ContentBrowserSettingsDesc& desc)
	{
		LOAD_FIELDS(CONTENT_BROWSER_SETTINGS_GENERATOR);
	}

	void PreferencesDocument::Load(TOMLNode node, TreeViewSettingsDesc& desc)
	{
		Load(node[detail::encode_key(desc.advanced_key)], desc.advanced);
	}

	void PreferencesDocument::Load(TOMLNode node, TreeViewAdvancedSettingsDesc& desc)
	{
		LOAD_FIELDS(TREE_VIEW_ADVANCED_SETTINGS_GENERATOR);
	}

	void PreferencesDocument::Load(TOMLNode node, FilesystemSettingsDesc& desc)
	{
		LOAD_FIELDS(FILESYSTEM_SETTINGS_GENERATOR);
	}

	void PreferencesDocument::Dump(toml::table& table, PreferencesDesc& desc)
	{
		toml::table subtable;

		subtable.clear();
		Dump(subtable, desc.edit);
		table.insert_or_assign(detail::encode_key(desc.edit_key), std::move(subtable));

		subtable.clear();
		Dump(subtable, desc.content_browser);
		table.insert_or_assign(detail::encode_key(desc.content_browser_key), std::move(subtable));

		subtable.clear();
		Dump(subtable, desc.tree_view);
		table.insert_or_assign(detail::encode_key(desc.tree_view_key), std::move(subtable));

		subtable.clear();
		Dump(subtable, desc.filesystem);
		table.insert_or_assign(detail::encode_key(desc.filesystem_key), std::move(subtable));
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

	void PreferencesDocument::Dump(toml::table& table, ContentBrowserSettingsDesc& desc)
	{
		DUMP_FIELDS(CONTENT_BROWSER_SETTINGS_GENERATOR);
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

	void PreferencesDocument::Dump(toml::table& table, FilesystemSettingsDesc& desc)
	{
		DUMP_FIELDS(FILESYSTEM_SETTINGS_GENERATOR);
	}
}
