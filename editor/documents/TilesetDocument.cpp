#include "TilesetDocument.h"

#include "core/windows/MainWindow.h"
#include "core/editor/Logger.h"

#include "gui/IDScope.h"
#include "gui/Subform.h"

#include "definitions/Keys.h"

namespace oly::editor
{
	const char* TilesetDocument::GetVersion()
	{
		return "1.0";
	}

	void TilesetDocument::Init()
	{
		if (!GetOlyPath().is_resource())
		{
			Notification notif(LogLevel::Warning, "Asset is not located in resource folder");
			MainWindow::Instance().PushNotification(std::move(notif));
		}

		Load();
	}

	void TilesetDocument::Draw()
	{
		gui::IDScope scope(this);
		if (auto form = Form())
		{
			ImGui::TableNextRow();
			Draw(form, _scratch);
		}
	}

	void TilesetDocument::DrawMenuBar()
	{
		// TODO v8 refactor to IDocument::DrawMenuBar() default
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
	}

	void TilesetDocument::Load()
	{
		if (_oly_path.exists())
		{
			_meta = detail::MetaSplitter::decode_meta(_oly_path);

			toml::table table;
			std::string err = _oly_path.load_toml(table);
			if (err.empty())
				Load(TOMLNode(table), _disk);
			else
			{
				Notification notif(LogLevel::Error, "cannot load tileset - corrupted asset: " + _oly_path.string());
				MainWindow::Instance().PushNotification(std::move(notif));
			}

			MarkClean();
		}
		else
		{
			Load(TOMLNode(), _disk);

			_meta = {};
			_meta.map[detail::Key::Meta_Version] = "1.0";
			_meta.map[detail::Key::Meta_Import] = "1";
			_meta.map[detail::Key::Meta_Type] = detail::encode_key(detail::Key::Meta_Tileset);

			MarkDirty();
		}

		_scratch = _disk;
	}

	void TilesetDocument::Dump()
	{
		toml::table table;
		Dump(table, _scratch);
		_oly_path.dump_toml(table, _meta);
		_disk = _scratch;
		MarkClean();
	}

	// TODO v8 use visual grid -> click on cells to get actual editor of fields

	void TilesetDocument::Draw(Form& form, TilesetDesc& desc)
	{
		DRAW_FIELDS(TILESET_PARTIAL_GENERATOR);

		for (auto& assignment : desc.assignments)
			Draw(form, assignment);
	}

	void TilesetDocument::Draw(Form& form, TilesetAssignmentDesc& desc)
	{
		DRAW_FIELD(texture); // TODO v8 support dropping paths externally or from tree view / content browser
		DRAW_FIELD(texture_index);
		DRAW_FIELD(config); // TODO v8 should be derived from grid
		DRAW_FIELD(uvs); // TODO v8 sublabels for x1/y1/x2/y2
		DRAW_FIELD(transformations); // TODO v8 should be bitflag checkbox
	}

	void TilesetDocument::Load(TOMLNode node, TilesetDesc& desc)
	{
		LOAD_FIELDS(TILESET_PARTIAL_GENERATOR);

		desc.assignments.Clear();
		if (auto array = node[detail::encode_key(desc.assignments_key)].as_array())
		{
			desc.assignments.vector.reserve(array->size());
			for (size_t i = 0; i < array->size(); ++i)
			{
				TilesetAssignmentDesc subdesc;
				Load(TOMLNode(array->get(i)), subdesc);
				desc.assignments.PushBack(std::move(subdesc));
			}
		}
	}

	void TilesetDocument::Load(TOMLNode node, TilesetAssignmentDesc& desc)
	{
		LOAD_FIELDS(TILESET_ASSIGNMENT_GENERATOR);
	}

	void TilesetDocument::Dump(toml::table& table, TilesetDesc& desc)
	{
		DUMP_FIELDS(TILESET_PARTIAL_GENERATOR);

		toml::array array;
		array.reserve(desc.assignments.Size());
		for (auto& subdesc : desc.assignments)
		{
			toml::table subtable;
			Dump(subtable, subdesc);
			array.push_back(std::move(subtable));
		}
		table.insert_or_assign(detail::encode_key(desc.assignments_key), std::move(array));
	}

	void TilesetDocument::Dump(toml::table& table, TilesetAssignmentDesc& desc)
	{
		DUMP_FIELDS(TILESET_ASSIGNMENT_GENERATOR);
	}
}
