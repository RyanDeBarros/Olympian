#include "TilesetDocument.h"

#include "core/windows/MainWindow.h"
#include "core/editor/Logger.h"
#include "core/Errors.h"

#include "gui/IDScope.h"
#include "gui/Subform.h"

#include "definitions/Keys.h"
#include "util/Parser.h"

namespace oly::editor
{
	IndividualEditorState::IndividualEditorState()
	{
		detail::TileConfigGrid g;
		for (size_t y = 0; y < 3; ++y)
			for (size_t x = 0; x < 3; ++x)
				g[y][x] = false;
		grid = g;
	}

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

		_individual_editor = {};
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
			if (ImGui::BeginChild("##Grid", ImVec2(0, 0), ImGuiChildFlags_Borders))
			{
				const float avail = std::min(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);
				const float cell_width = 0.2f * avail;
				const ImVec2 cell_size(cell_width, cell_width);
				const ImVec2 cursor = ImGui::GetCursorScreenPos();

				for (detail::GridCoordinate y = detail::GridCoordinate::Top; y <= detail::GridCoordinate::Bottom; ++y)
				{
					for (detail::GridCoordinate x = detail::GridCoordinate::Left; x <= detail::GridCoordinate::Right; ++x)
					{
						gui::IDScope scope;
						scope.Push(y);
						scope.Push(x);
						const ImVec2 rect_start = cursor + cell_size * ImVec2(x + 1, y + 1);
						const ImVec2 rect_end = rect_start + cell_size;

						if (y == 1 && x == 1)
						{
							if (_individual_editor.texture_error)
								ImGui::GetWindowDrawList()->AddRectFilled(rect_start, rect_end, IM_COL32(255, 0, 255, 255));
							else if (_individual_editor.active_texture.Empty())
								ImGui::GetWindowDrawList()->AddRectFilled(rect_start, rect_end, IM_COL32(16, 16, 16, 255));
							else
							{
								UVRect uvs = GetAssignment(_individual_editor.grid).uvs.scratch;
								ImGui::GetWindowDrawList()->AddImage(_individual_editor.active_texture.ID(), rect_start, rect_end, ImVec2(uvs.x1, uvs.y1), ImVec2(uvs.x2, uvs.y2));
							}
						}
						else
							_individual_editor.stale_texture |= DrawToggleCell(rect_start, rect_end, _individual_editor.grid[y][x], detail::tile_config_is_available(x, y, _individual_editor.grid));
					}
				}

				if (_individual_editor.stale_texture)
				{
					_individual_editor.stale_texture = false;
					_individual_editor.texture_error = false;
					auto& desc = GetAssignment(_individual_editor.grid);
					if (desc.texture.scratch.empty())
						_individual_editor.active_texture = {};
					else
					{
						BreakoutError::NotifyScope notify(true);
						try
						{
							_individual_editor.active_texture.LoadGeneric(detail::ResourcePath(desc.texture.scratch).string());
						}
						catch (const BreakoutError& e)
						{
							_individual_editor.texture_error = true;
						}
					}
				}
			}
			ImGui::EndChild();

			ImGui::TableNextColumn();
			if (ImGui::BeginChild("##Desc", ImVec2(0, 0), ImGuiChildFlags_Borders))
				Draw(GetAssignment(_individual_editor.grid));
			ImGui::EndChild();

			ImGui::EndTable();
		}
	}

	bool TilesetDocument::DrawToggleCell(ImVec2 rect_start, ImVec2 rect_end, bool& on, const bool available)
	{
		bool grid_changed = false;

		if (available)
			ImGui::GetWindowDrawList()->AddRectFilled(rect_start, rect_end, on ? IM_COL32(0, 127, 255, 255) : IM_COL32(64, 64, 64, 255));
		else
		{
			on = false;
			ImGui::GetWindowDrawList()->AddRectFilled(rect_start, rect_end, IM_COL32(32, 32, 32, 255));
		}

		ImGui::GetWindowDrawList()->AddRect(rect_start, rect_end, IM_COL32_BLACK, 0.f, 0, 2.f);

		if (available)
		{
			ImGui::SetCursorScreenPos(rect_start);
			if (ImGui::InvisibleButton("##Click", rect_end - rect_start))
			{
				on = !on;
				grid_changed = true;
			}

			if (ImGui::IsItemHovered())
				ImGui::GetWindowDrawList()->AddRectFilled(rect_start, rect_end, ImGui::GetColorU32(IM_COL32_WHITE, 0.3f));
		}

		return grid_changed;
	}

	void TilesetDocument::Draw(TilesetAssignmentDesc& desc)
	{
		if (auto form = Form())
		{
			DRAW_FIELD(texture); // TODO v8 support dropping paths externally or from tree view / content browser
			DRAW_FIELD(texture_index);
			//DRAW_FIELD(config); // TODO v8 should be derived from grid
			DRAW_FIELD(uvs); // TODO v8 sublabels for x1/y1/x2/y2
			DRAW_FIELD(transformations); // TODO v8 should be two combos (reflect + rotate)
		}
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

	TilesetAssignmentDesc& TilesetDocument::GetAssignment(const detail::TileConfigGrid grid)
	{
		return _scratch.assignments.map[detail::tile_config_from_grid(grid)];
	}
}
