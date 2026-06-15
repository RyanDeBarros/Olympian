#include "TilesetDocument.h"

#include "core/windows/MainWindow.h"
#include "core/editor/Logger.h"

#include "gui/IDScope.h"
#include "gui/Subform.h"

#include "definitions/Keys.h"
#include "util/Parser.h"

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

		if (auto section = CollapsingSection("Advanced"))
		{
			if (auto form = Form())
			{
				if (_scratch.storage.Draw())
					MarkDirty();
			}
		}

		if (ImGui::BeginTabBar("##Editors"))
		{
			if (ImGui::BeginTabItem("Group"))
			{
				DrawGroupEditor();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Individual"))
			{
				DrawIndividualEditor();
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
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

	void TilesetDocument::DrawGroupEditor()
	{
		// TODO v8
	}

	void TilesetDocument::DrawIndividualEditor()
	{
		if (ImGui::BeginTable("##Table", 2))
		{
			ImGui::TableNextColumn();
			// TODO v8

			ImGui::TableNextColumn();
			// TODO v8

			ImGui::EndTable();
		}
	}

	void TilesetDocument::Draw(Form& form, TilesetAssignmentDesc& desc)
	{
		DRAW_FIELD(texture); // TODO v8 support dropping paths externally or from tree view / content browser
		DRAW_FIELD(texture_index);
		//DRAW_FIELD(config); // TODO v8 should be derived from grid
		DRAW_FIELD(uvs); // TODO v8 sublabels for x1/y1/x2/y2
		DRAW_FIELD(transformations); // TODO v8 should be two combos (reflect + rotate)
	}

	void TilesetDocument::Load(TOMLNode node, TilesetDesc& desc)
	{
		LOAD_FIELDS(TILESET_PARTIAL_GENERATOR);

		desc.assignments.map.Clear();
		if (auto table = node[detail::encode_key(desc.assignments_key)].as_table())
		{
			for (auto&& [key, node] : *table)
			{
				if (auto config = detail::stoi(key.str()))
				{
					TilesetAssignmentDesc subdesc;
					Load(TOMLNode(node), subdesc);
					desc.assignments.map[*config] = std::move(subdesc);
				}
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

		toml::table subtable;
		for (auto& [config, subdesc] : desc.assignments.map)
		{
			toml::table dump;
			Dump(dump, subdesc);
			subtable.insert_or_assign(std::to_string(config), std::move(dump));
		}
		table.insert_or_assign(detail::encode_key(desc.assignments_key), std::move(subtable));
	}

	void TilesetDocument::Dump(toml::table& table, TilesetAssignmentDesc& desc)
	{
		DUMP_FIELDS(TILESET_ASSIGNMENT_GENERATOR);
	}
}
