#include "TilesetDocument.h"

#include "core/windows/MainWindow.h"
#include "core/editor/Logger.h"
#include "core/editor/UID.h"
#include "core/Errors.h"

#include "gui/IDScope.h"
#include "gui/Subform.h"

#include "definitions/Keys.h"
#include "util/Parser.h"

namespace oly::editor
{
	Grid4x4EditorState::Grid4x4EditorState() :
		grid_1x1(std::array<std::array<bool, 1>, 1>{
			std::array<bool, 1>{ true }
		}),
		grid_3x1(std::array<std::array<bool, 3>, 1>{
			std::array<bool, 3>{ true, true, true }
		}),
		grid_1x3(std::array<std::array<bool, 1>, 3>{
			std::array<bool, 1>{ true },
			std::array<bool, 1>{ true },
			std::array<bool, 1>{ true }
		}),
		grid_3x3(std::array<std::array<bool, 3>, 3>{
			std::array<bool, 3>{ true, true, true },
			std::array<bool, 3>{ true, true, true },
			std::array<bool, 3>{ true, true, true }
		})
	{
	}

	const detail::TileConfigGrid* Grid4x4EditorState::At(int x, int y) const
	{
		if (0 <= x && x <= 2)
		{
			if (0 <= y && y <= 2)
				return grid_3x3.At(x, y);
			else if (y == 4)
				return grid_3x1.At(x, y - 4);
			else
				return nullptr;
		}
		else if (x == 4)
		{
			if (0 <= y && y <= 2)
				return grid_1x3.At(x - 4, y);
			else if (y == 4)
				return grid_1x1.At(x - 4, y - 4);
			else
				return nullptr;
		}
		else
			return nullptr;
	}

	detail::TileConfigGrid* Grid4x4EditorState::At(int x, int y)
	{
		if (0 <= x && x <= 2)
		{
			if (0 <= y && y <= 2)
				return grid_3x3.At(x, y);
			else if (y == 4)
				return grid_3x1.At(x, y - 4);
			else
				return nullptr;
		}
		else if (x == 4)
		{
			if (0 <= y && y <= 2)
				return grid_1x3.At(x - 4, y);
			else if (y == 4)
				return grid_1x1.At(x - 4, y - 4);
			else
				return nullptr;
		}
		else
			return nullptr;
	}

	size_t Grid4x4EditorState::Rows() const
	{
		return 7;
	}

	size_t Grid4x4EditorState::Cols() const
	{
		return 7;
	}

	Grid5x5EditorState::Grid5x5EditorState() :
		grid_5x5(std::array<std::array<bool, 5>, 5>{
			std::array<bool, 5>{ true, true,  true, true,  true },
			std::array<bool, 5>{ true, false, true, false, true },
			std::array<bool, 5>{ true, true,  true, true,  true },
			std::array<bool, 5>{ true, false, true, false, true },
			std::array<bool, 5>{ true, true,  true, true,  true }
		})
	{
	}

	const detail::TileConfigGrid* Grid5x5EditorState::At(int x, int y) const
	{
		if (0 <= x && x <= 4 && 0 <= y && y <= 4)
			return grid_5x5.At(x, y);
		else
			return nullptr;
	}

	detail::TileConfigGrid* Grid5x5EditorState::At(int x, int y)
	{
		if (0 <= x && x <= 4 && 0 <= y && y <= 4)
			return grid_5x5.At(x, y);
		else
			return nullptr;
	}

	size_t Grid5x5EditorState::Rows() const
	{
		return 5;
	}

	size_t Grid5x5EditorState::Cols() const
	{
		return 5;
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
		_group_editors = {};
		Load();
	}

	void TilesetDocument::Draw()
	{
		for (auto& [config, active] : _textures)
		{
			if (!active.stale)
				continue;

			active.stale = false;
			active.error = false;
			auto& desc = GetAssignment(config);
			if (desc.texture.scratch.empty())
				active.texture = {};
			else
			{
				BreakoutError::NotifyScope notify(true);
				try
				{
					active.texture.LoadGeneric(detail::ResourcePath(desc.texture.scratch).string());
				}
				catch (const BreakoutError& e)
				{
					active.error = true;
				}
			}
		}

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
		int type_index = static_cast<int>(_group_editors.current_type);
		ImGui::Text("Grid Type"); ImGui::SameLine();
		if (gui::Combo("##GridType", type_index, { "Standard 4x4", "Standard 5x5" }))
			_group_editors.current_type = static_cast<GroupEditorType>(type_index);

		GridEditorStateBase* editor = nullptr;
		switch (_group_editors.current_type)
		{
		case GroupEditorType::T4x4:
			editor = &_group_editors.state_4x4;
			break;

		case GroupEditorType::T5x5:
			editor = &_group_editors.state_5x5;
			break;
		}

		if (editor)
		{
			if (ImGui::BeginTable("##Table", 2))
			{
				ImGui::TableNextColumn();
				bool new_cell_selected = false;
				if (ImGui::BeginChild("##Grid", ImVec2(0, 0), ImGuiChildFlags_Borders))
				{
					const float avail_cell_width = ImGui::GetContentRegionAvail().x / (editor->Cols() + 2.f);
					const float avail_cell_height = ImGui::GetContentRegionAvail().y / (editor->Rows() + 2.f);
					const float cell_side = std::min(avail_cell_width, avail_cell_height);
					const ImVec2 cell_size(cell_side, cell_side);
					const ImVec2 cursor = ImGui::GetCursorScreenPos();
					// TODO v8 size or offset is not working for 4x4
					const ImVec2 offset = 0.5f * (ImGui::GetContentRegionAvail() - cell_side * ImVec2(editor->Cols() + 2.f, editor->Rows() + 2.f));

					for (int y = 0; y < editor->Rows(); ++y)
					{
						for (int x = 0; x < editor->Cols(); ++x)
						{
							if (const detail::TileConfigGrid* grid = editor->At(x, y))
							{
								gui::IDScope scope;
								scope.Push(y);
								scope.Push(x);
								const ImVec2 rect_start = cursor + offset + cell_size * ImVec2(x + 1, y + 1);
								const ImVec2 rect_end = rect_start + cell_size;

								DrawActiveTexture(rect_start, rect_end, *grid, 64);
								auto cell = std::make_pair(x, y);

								ImGui::SetCursorScreenPos(rect_start);
								if (ImGui::InvisibleButton("##ToggleSelected", cell_size))
								{
									new_cell_selected = true;
									if (editor->selected_cell == cell)
										editor->selected_cell = std::nullopt;
									else
										editor->selected_cell = cell;
								}

								if (ImGui::IsItemHovered())
									ImGui::GetWindowDrawList()->AddRectFilled(rect_start, rect_end, ImGui::GetColorU32(IM_COL32_WHITE, 0.3f));

								if (editor->selected_cell == cell)
									ImGui::GetWindowDrawList()->AddRect(rect_start, rect_end, IM_COL32(0, 255, 0, 255), 0.f, 0, 4.f);
							}
						}
					}
				}
				ImGui::EndChild();
				if (ImGui::IsItemClicked() && !new_cell_selected)
					editor->selected_cell = std::nullopt;

				ImGui::TableNextColumn();
				if (ImGui::BeginChild("##Desc", ImVec2(0, 0), ImGuiChildFlags_Borders))
				{
					if (auto cell = editor->selected_cell)
						if (auto grid = editor->At(cell->first, cell->second))
							Draw(*grid);
				}
				ImGui::EndChild();

				ImGui::EndTable();
			}
		}
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
				const ImVec2 offset = 0.5f * (ImGui::GetContentRegionAvail() - ImVec2(avail, avail));

				for (detail::GridCoordinate y = detail::GridCoordinate::Top; y <= detail::GridCoordinate::Bottom; ++y)
				{
					for (detail::GridCoordinate x = detail::GridCoordinate::Left; x <= detail::GridCoordinate::Right; ++x)
					{
						gui::IDScope scope;
						scope.Push(y);
						scope.Push(x);
						const ImVec2 rect_start = cursor + offset + cell_size * ImVec2(x + 1, y + 1);
						const ImVec2 rect_end = rect_start + cell_size;

						if (y == 1 && x == 1)
							DrawActiveTexture(rect_start, rect_end, _individual_editor.grid, 16);
						else
							DrawToggleCell(rect_start, rect_end, _individual_editor.grid[y][x], detail::tile_config_is_available(x, y, _individual_editor.grid));
					}
				}
			}
			ImGui::EndChild();

			ImGui::TableNextColumn();
			if (ImGui::BeginChild("##Desc", ImVec2(0, 0), ImGuiChildFlags_Borders))
				Draw(_individual_editor.grid);
			ImGui::EndChild();

			ImGui::EndTable();
		}
	}

	void TilesetDocument::DrawToggleCell(ImVec2 rect_start, ImVec2 rect_end, bool& on, const bool available)
	{
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
				on = !on;

			if (ImGui::IsItemHovered())
				ImGui::GetWindowDrawList()->AddRectFilled(rect_start, rect_end, ImGui::GetColorU32(IM_COL32_WHITE, 0.3f));
		}
	}

	void TilesetDocument::Draw(const detail::TileConfigGrid grid)
	{
		if (auto form = Form())
		{
			TilesetAssignmentDesc& desc = GetAssignment(grid);

			{
				DescIO::PrepareValue(desc.texture.label);
				gui::IDScope scope(&desc.texture.scratch);
				
				if (gui::InputData<std::string>{}("", desc.texture.scratch))
					MarkDirty();

				if (ImGui::IsItemDeactivatedAfterEdit())
					OnActiveTextureChanged(grid);

				if (ImGui::BeginDragDropTarget())
				{
					if (auto payload = ImGui::AcceptDragDropPayload(StringID(UID::PathDrag)))
					{
						detail::ResourcePath path(std::string_view(reinterpret_cast<const char*>(payload->Data), payload->DataSize));
						if (path.is_resource())
						{
							desc.texture.scratch = path.get_resource_shorthand();
							MarkDirty();
							OnActiveTextureChanged(grid);
						}
						else
							MainWindow::Instance().PushNotification(Notification(LogLevel::Error, "Path is not located in resource folder"));
					}

					ImGui::EndDragDropTarget();
				}

				if (DescIO::CheckRevertButton(desc.texture.scratch, desc.texture.def))
				{
					MarkDirty();
					OnActiveTextureChanged(grid);
				}
			}

			DRAW_FIELD(texture_index);
			DRAW_FIELD(uvs);
			DRAW_FIELD(reflection);
			DRAW_FIELD(rotation);
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
		return GetAssignment(detail::tile_config_from_grid(grid));
	}

	TilesetAssignmentDesc& TilesetDocument::GetAssignment(const detail::TileConfig config)
	{
		return _scratch.assignments.map[config];
	}

	void TilesetDocument::OnActiveTextureChanged(const detail::TileConfigGrid grid)
	{
		GetActiveTexture(grid).stale = true;
	}

	void TilesetDocument::DrawActiveTexture(ImVec2 rect_start, ImVec2 rect_end, const detail::TileConfigGrid grid, unsigned char empty_gray_value)
	{
		auto& active = GetActiveTexture(grid);
		if (active.error)
			ImGui::GetWindowDrawList()->AddRectFilled(rect_start, rect_end, IM_COL32(255, 0, 255, 255));
		else if (active.texture.Empty())
			ImGui::GetWindowDrawList()->AddRectFilled(rect_start, rect_end, IM_COL32(empty_gray_value, empty_gray_value, empty_gray_value, 255));
		else
		{
			UVRect uvs = GetAssignment(grid).uvs.scratch;
			ImGui::GetWindowDrawList()->AddImage(active.texture.ID(), rect_start, rect_end, ImVec2(uvs.x1, uvs.y1), ImVec2(uvs.x2, uvs.y2));
		}
	}

	TilesetDocument::ActiveTexture& TilesetDocument::GetActiveTexture(const detail::TileConfigGrid grid)
	{
		return _textures[detail::tile_config_from_grid(grid)];
	}
}
