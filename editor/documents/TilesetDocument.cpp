#include "TilesetDocument.h"

#include "core/windows/MainWindow.h"
#include "core/editor/Logger.h"
#include "core/editor/UID.h"
#include "core/Colors.h"
#include "core/Errors.h"

#include "gui/scopes/IDScope.h"
#include "gui/scopes/Subform.h"
#include "gui/graphics/Overlays.h"

#include "documents/TextureDocument.h"

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
		return 5;
	}

	size_t Grid4x4EditorState::Cols() const
	{
		return 5;
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

	void TilesetDocument::InitImpl()
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
		auto pre_draw = PreDraw();

		UpdateActiveTextures();
		gui::IDScope scope(this);

		DataPathSource path;

		if (auto subform = Subform("Advanced"))
			_scratch.storage.Draw(path / _scratch.subpaths.storage);

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
		if (_oly_path.is_file())
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

	void* TilesetDocument::VisitPath(DataPath path, std::type_index type)
	{
		return _scratch.VisitPath(path, type);
	}

	void TilesetDocument::DrawGroupEditor()
	{
		int type_index = static_cast<int>(_group_editors.current_type);
		if (gui::Combo("Grid type", type_index, { "Standard 4x4", "Standard 5x5" }))
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

		if (!editor)
			return;

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
				const ImVec2 offset = 0.5f * (ImGui::GetContentRegionAvail() - cell_side * ImVec2(editor->Cols() + 2.f, editor->Rows() + 2.f));

				for (int y = 0; y < editor->Rows(); ++y)
				{
					for (int x = 0; x < editor->Cols(); ++x)
					{
						const detail::TileConfigGrid* grid = editor->At(x, y);
						if (!grid)
							continue;

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
						{
							ImGui::GetWindowDrawList()->AddRectFilled(rect_start, rect_end, ImGui::GetColorU32(Color::White, 0.3f));
							TextureErrorTooltip(GetActiveTexture(*grid).error);
						}

						if (editor->selected_cell == cell)
							ImGui::GetWindowDrawList()->AddRect(rect_start, rect_end, Color::Green, 0.f, 0, 4.f);
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
						{
							DrawActiveTexture(rect_start, rect_end, _individual_editor.grid, 16);
							ImGui::Dummy(cell_size);
						}
						else
							DrawToggleCell(rect_start, rect_end, _individual_editor.grid[y][x], detail::tile_config_is_available(x, y, _individual_editor.grid));

						if (ImGui::IsItemHovered())
							TextureErrorTooltip(GetActiveTexture(_individual_editor.grid).error);
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
			ImGui::GetWindowDrawList()->AddRectFilled(rect_start, rect_end, on ? Color::Azure : Color::Grey(64));
		else
		{
			on = false;
			ImGui::GetWindowDrawList()->AddRectFilled(rect_start, rect_end, Color::Grey(32));
		}

		ImGui::GetWindowDrawList()->AddRect(rect_start, rect_end, Color::Black, 0.f, 0, 2.f);

		ImGui::SetCursorScreenPos(rect_start);
		if (available)
		{
			if (ImGui::InvisibleButton("##Click", rect_end - rect_start))
				on = !on;

			if (ImGui::IsItemHovered())
				ImGui::GetWindowDrawList()->AddRectFilled(rect_start, rect_end, ImGui::GetColorU32(Color::White, 0.3f));
		}
		else
			ImGui::Dummy(rect_end - rect_start);
	}

	void TilesetDocument::Draw(const detail::TileConfigGrid grid)
	{
		if (auto form = Form())
		{
			TilesetAssignmentDesc& desc = GetAssignment(grid);

			if (auto scope = gui::IDScope(&desc.texture))
			{
				gui::PropertyGrid::Key::SetLabel(desc.texture.label);
				if (desc.texture.scratch != desc.texture.def)
					gui::PropertyGrid::Reset::Button();

				gui::PropertyGrid::Value::AddComponent(comp::Generic([this, &desc, grid]() -> DrawResult {
					gui::IDScope scope(&desc.texture.scratch);
					DrawResult result;

					result |= gui::InputData<std::string>{}("", desc.texture.scratch);

					if (ImGui::IsItemDeactivatedAfterEdit()) // TODO v9.1 this doesn't trigger when closing window or switching tabs, etc.
						OnActiveTextureChanged(grid);

					if (ImGui::BeginDragDropTarget())
					{
						if (auto payload = ImGui::AcceptDragDropPayload(StringID(UID::PathDrag)))
						{
							detail::ResourcePath path(std::string_view(reinterpret_cast<const char*>(payload->Data), payload->DataSize));
							if (path.is_resource())
							{
								desc.texture.scratch = path.get_resource_shorthand();
								result.SetDirty(true);
								OnActiveTextureChanged(grid);
							}
							else
								MainWindow::Instance().PushNotification(Notification(LogLevel::Error, "Path is not located in resource folder"));
						}

						ImGui::EndDragDropTarget();
					}

					return result;
				}));

				gui::PropertyGrid::SubmitRow();
				if (gui::PropertyGrid::Reset::AnyActivated())
				{
					// TODO v9.1 push undo actions for all manual document drawing outside of field system
					desc.texture.scratch = desc.texture.def;
					OnActiveTextureChanged(grid);
				}
			}

			auto path = GetAssignmentPath(grid);
			desc.texture_index.Draw(path / desc.subpaths.texture_index);
			if (gui::PropertyGrid::DirtyRow())
				OnActiveTextureChanged(grid);

			desc.uvs.Draw(path / desc.subpaths.uvs);
			desc.reflection.Draw(path / desc.subpaths.reflection);
			desc.rotation.Draw(path / desc.subpaths.rotation);
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
				if (auto config = stoi(key.str()))
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
		return GetAssignment(GetResolvedTileConfig(grid));
	}

	TilesetAssignmentDesc& TilesetDocument::GetAssignment(const detail::TileConfig config)
	{
		return _scratch.assignments.map[config];
	}

	DataPathSource TilesetDocument::GetAssignmentPath(const detail::TileConfigGrid grid)
	{
		return GetAssignmentPath(GetResolvedTileConfig(grid));
	}

	DataPathSource TilesetDocument::GetAssignmentPath(const detail::TileConfig config)
	{
		DataPathSource path;
		path /= _scratch.subpaths.assignments;
		path /= _scratch.assignments.subpaths.map;
		return _scratch.assignments.map.Subpath(path, config);
	}

	void TilesetDocument::OnActiveTextureChanged(const detail::TileConfigGrid grid)
	{
		GetActiveTexture(grid).stale = true;
	}

	void TilesetDocument::UpdateActiveTextures()
	{
		for (auto& [config, active] : _textures)
		{
			if (!active.stale)
				continue;

			active.stale = false;
			active.error = TextureError::None;
			auto& desc = GetAssignment(config);
			if (desc.texture.scratch.empty())
				active.texture = {};
			else
			{
				BreakoutError::NotifyScope notify(true);
				try
				{
					std::string filepath = detail::ResourcePath(desc.texture.scratch).string();
					GLenum min_filter, mag_filter;
					float scale = 1.f;
					bool generate_mipmaps = false;
					auto result = TextureDocument::LoadTextureSettings(filepath, desc.texture_index.scratch, min_filter, mag_filter, scale, generate_mipmaps);
					if (result == TextureDocument::TextureSettingsLoadResult::Success)
						active.texture = Texture::LoadGeneric(filepath, min_filter, mag_filter, scale, generate_mipmaps);
					else
					{
						switch (result)
						{
						case TextureDocument::TextureSettingsLoadResult::NotAFile:
							active.error = TextureError::NotAFile;
							break;
						case TextureDocument::TextureSettingsLoadResult::NotAResource:
							active.error = TextureError::NotAResource;
							break;
						case TextureDocument::TextureSettingsLoadResult::MissingImport:
							active.error = TextureError::MissingImport;
							break;
						case TextureDocument::TextureSettingsLoadResult::NotATexture:
							active.error = TextureError::NotATexture;
							break;
						case TextureDocument::TextureSettingsLoadResult::BadSlot:
							active.error = TextureError::BadSlot;
							break;
						case TextureDocument::TextureSettingsLoadResult::Corrupted:
						default:
							active.error = TextureError::CorruptedAsset;
							break;
						}
					}
				}
				catch (const BreakoutError& e)
				{
					active.error = TextureError::CannotLoad;
				}
			}
		}
	}

	bool TilesetDocument::TextureErrorIsWarning(TextureError error)
	{
		return error == TextureError::BadSlot || error == TextureError::NotAResource;
	}

	void TilesetDocument::DrawActiveTexture(ImVec2 rect_start, ImVec2 rect_end, const detail::TileConfigGrid grid, unsigned char empty_gray_value)
	{
		auto& active = GetActiveTexture(grid);
		if (active.error != TextureError::None && !TextureErrorIsWarning(active.error))
			gui::Overlay::QuadError(rect_start, rect_end);
		else if (active.error != TextureError::None && TextureErrorIsWarning(active.error))
		{
			DrawActiveTextureDirect(grid, rect_start, rect_end);
			gui::Overlay::QuadWarning(rect_start, rect_end);
		}
		else if (active.texture.Empty())
			ImGui::GetWindowDrawList()->AddRectFilled(rect_start, rect_end, Color::Grey(empty_gray_value));
		else
			DrawActiveTextureDirect(grid, rect_start, rect_end);
	}

	void TilesetDocument::DrawActiveTextureDirect(const detail::TileConfigGrid grid, ImVec2 rect_start, ImVec2 rect_end)
	{
		auto& desc = GetAssignment(grid);
		auto& active = GetActiveTexture(grid);
		auto uv_rect = desc.uvs.scratch;
		detail::TileReflection reflection = desc.reflection.scratch;
		detail::TileRotation rotation = desc.rotation.scratch;

		std::array<ImVec2, 4> uvs;
		uvs[0] = ImVec2(uv_rect.x1, uv_rect.y1);
		uvs[1] = ImVec2(uv_rect.x2, uv_rect.y1);
		uvs[2] = ImVec2(uv_rect.x2, uv_rect.y2);
		uvs[3] = ImVec2(uv_rect.x1, uv_rect.y2);

		if (static_cast<bool>(reflection & detail::TileReflection::X))
		{
			std::swap(uvs[0], uvs[1]);
			std::swap(uvs[2], uvs[3]);
		}

		if (static_cast<bool>(reflection & detail::TileReflection::Y))
		{
			std::swap(uvs[0], uvs[3]);
			std::swap(uvs[1], uvs[2]);
		}

		{
			std::array<ImVec2, 4> temp;
			for (int i = 0; i < 4; ++i)
				temp[i] = uvs[static_cast<size_t>((i + static_cast<int>(rotation)) % 4)];
			uvs = temp;
		}

		ImVec2 rect_delta = rect_end - rect_start;
		ImGui::GetWindowDrawList()->AddImageQuad(active.texture.ID(), rect_start, rect_start + ImVec2(rect_delta.x, 0.f),
			rect_start + rect_delta, rect_start + ImVec2(0.f, rect_delta.y), uvs[0], uvs[1], uvs[2], uvs[3]);
	}

	void TilesetDocument::TextureErrorTooltip(TextureError error)
	{
		switch (error)
		{
		case TextureError::NotAFile:
			ImGui::SetTooltip("Cannot find texture file");
			break;
		case TextureError::NotAResource:
			ImGui::SetTooltip("Texture is not located in resource folder");
			break;
		case TextureError::MissingImport:
			ImGui::SetTooltip("Texture is missing import file");
			break;
		case TextureError::NotATexture:
			ImGui::SetTooltip("File is not imported as a texture");
			break;
		case TextureError::CorruptedAsset:
			ImGui::SetTooltip("Import file is corrupted");
			break;
		case TextureError::BadSlot:
			ImGui::SetTooltip("Bad slot index");
			break;
		case TextureError::CannotLoad:
			ImGui::SetTooltip("Editor cannot load texture");
			break;
		}
	}

	TilesetDocument::ActiveTexture& TilesetDocument::GetActiveTexture(const detail::TileConfigGrid grid)
	{
		return _textures[GetResolvedTileConfig(grid)];
	}

	detail::TileConfig TilesetDocument::GetResolvedTileConfig(const detail::TileConfigGrid grid)
	{
		auto config = detail::tile_config_from_grid(grid);
		detail::simplify_tile_config(config);
		return config;
	}
}
