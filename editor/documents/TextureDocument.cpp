#include "TextureDocument.h"

#include "definitions/Keys.h"

#include "core/MainWindow.h"
#include "core/Logger.h"

#include "core/ResourceLoader.h"
#include "graphics/Toolbar.h"

#include <imgui.h>

namespace oly::editor
{
	const char* TextureDocument::GetVersion()
	{
		return "1.0";
	}

	void TextureDocument::Init()
	{
		_gif = GetSourcePath().extension_matches(".gif");
		_svg = GetSourcePath().extension_matches(".svg");
		Load();

		if (_svg)
			_texture = { SVGTexture(GetSourcePath().string().c_str()) };
		else if (_gif)
			_texture = { GIFTexture(GetSourcePath().string().c_str()) };
		else
			_texture = { RasterTexture(GetSourcePath().string().c_str()) };
	}

	void TextureDocument::Draw()
	{
		ImGui::PushID(this);
		if (ImGui::BeginTable("", 2))
		{
			ImGui::TableNextColumn();
			Draw(_scratch);

			ImGui::TableNextColumn();
			DrawPreview();
			ImGui::EndTable();
		}
		ImGui::PopID();
	}

	void TextureDocument::Load()
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
				Notification notif(LogLevel::Error, "cannot load texture - corrupted asset: " + GetSourcePath().string());
				MainWindow::Instance().PushNotification(std::move(notif));
			}
		}
		else
		{
			Load(TOMLNode(), _disk);

			_meta = {};
			_meta.map[detail::Key::Meta_Version] = "1.0";
			_meta.map[detail::Key::Meta_Import] = "1";
			_meta.map[detail::Key::Meta_Type] = detail::encode_key(detail::Key::Meta_Texture);
		}

		_scratch.Reset(_disk);
	}

	void TextureDocument::Dump()
	{
		toml::table table;
		Dump(table, _scratch);
		_oly_path.dump_toml(table, _meta);
		_disk.Reset(_scratch);
		MarkClean();
	}

	detail::ResourcePath TextureDocument::GetSourcePath() const
	{
		return _oly_path.get_source_path();
	}

	void TextureDocument::DrawPreview()
	{
		if (ImGui::BeginChild("Preview", ImVec2(0, 0), ImGuiChildFlags_Borders))
		{
			ImGui::Text("Preview");
			ImGui::Separator();
			if (Toolbar::DrawIconButton(Resource::RecenterIcon, "Reset panning/zoom", &_preview_nav))
			{
				_preview_nav = {};
				if (SVGTexture* svg = _texture.GetSVG())
					_texture = { SVGTexture(GetSourcePath().string().c_str(), _preview_nav.svg_scale) };
			}
			
			if (GIFTexture* gif = _texture.GetGIF())
			{
				ImGui::SameLine();
				ImGui::Text("Speed");
				ImGui::SameLine();
				ImGui::SetNextItemWidth(100.0f);
				ImGui::InputFloat("##SpeedInput", &gif->speed);
				gif->Update(ImGui::GetIO().DeltaTime);
			}

			if (SVGTexture* svg = _texture.GetSVG())
			{
				ImGui::SameLine();
				ImGui::Text("Scale");
				ImGui::SameLine();
				ImGui::SetNextItemWidth(100.0f);
				float scale = svg->preview_scale * _preview_nav.svg_scale;
				ImGui::InputFloat("##ScaleInput", &scale);
				svg->preview_scale = scale / _preview_nav.svg_scale;
				ImGui::SameLine();
				if (Toolbar::DrawIconButton(Resource::RefreshIcon, "Refresh SVG scale", &svg->preview_scale))
				{
					_preview_nav.svg_scale = scale;
					_texture = { SVGTexture(GetSourcePath().string().c_str(), _preview_nav.svg_scale) };
				}
			}

			if (ImGui::IsWindowHovered())
			{
				_preview_nav.zoom += ImGui::GetIO().MouseWheel;

				if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
					_preview_nav.pos += ImGui::GetIO().MouseDelta;
			}

			ImVec2 avail = ImGui::GetContentRegionAvail();
			ImVec2 cursor = ImGui::GetCursorScreenPos();
			ImVec2 size = _texture.Size() * std::pow(2.f, _preview_nav.zoom);
			
			ImVec2 offset = 0.5f * (avail - size) + _preview_nav.pos;
			ImVec2 pos = cursor + offset;

			ImGui::GetWindowDrawList()->AddImage(_texture.ID(), pos, pos + size);
			if (!_gif)
				DrawSpritesheetOverlay(pos);
			ImGui::EndChild();
		}
	}

	void TextureDocument::DrawSpritesheetOverlay(ImVec2 rect_start)
	{
		SpritesheetDesc* desc = nullptr;
		if (auto d = _scratch.Visit(_active_slot, [](auto& desc) -> SpritesheetDesc* { return desc.base.anim.scratch ? &desc.base.spritesheet : nullptr; }))
			desc = *d;

		if (!desc)
			return;

		auto dl = ImGui::GetWindowDrawList();
		ImU32 line_color = IM_COL32_WHITE;
		float line_thickness = 1.f;

		const ImVec2 texture_size = _texture.Size() * _preview_nav.svg_scale * std::pow(2.f, _preview_nav.zoom);

		int cols = desc->cols.scratch;
		float cell_width = desc->cell_width_override.scratch;

		if (desc->enable_cell_width_override.scratch)
			cols = static_cast<int>(texture_size.x) / static_cast<int>(cell_width);
		else
			cell_width = texture_size.x / cols;

		const float full_width = cols * cell_width;

		int rows = desc->rows.scratch;
		float cell_height = desc->cell_height_override.scratch;

		if (desc->enable_cell_height_override.scratch)
			rows = static_cast<int>(texture_size.y) / static_cast<int>(cell_height);
		else
			cell_height = texture_size.y / rows;

		const float full_height = rows * cell_height;

		for (int i = 0; i <= cols; ++i)
		{
			int x = i * full_width / cols;
			dl->AddLine(rect_start + ImVec2(x, 0), rect_start + ImVec2(x, full_height), line_color, line_thickness);
		}

		for (int i = 0; i <= rows; ++i)
		{
			int y = i * full_height / rows;
			dl->AddLine(rect_start + ImVec2(0, y), rect_start + ImVec2(full_width, y), line_color, line_thickness);
		}

		// TODO v8 put numbers in cell corners using row_up/row_major
		// TODO v8 animate 'active' cell using delay_cs
	}

	void TextureDocument::Draw(TextureDescVariant& desc)
	{
		if (DescIO::BeginForm(this))
		{
			DescIO::FormSeparator(this, "General");
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Text("Select Slot");

			ImGui::SameLine();
			static const char PLUS_ID = 0;
			if (Toolbar::DrawIconButton(Resource::PlusIcon, "New texture slot", &PLUS_ID))
			{
				_active_slot = _slot_names.size();
				_scratch.PushBack();
				MarkDirty();
			}

			ImGui::SameLine();
			static const char MINUS_ID = 0;
			if (Toolbar::DrawIconButton(Resource::MinusIcon, "Remove texture slot", &MINUS_ID))
			{
				_scratch.Remove(_active_slot);
				if (_scratch.Empty())
					_scratch.PushBack();
				MarkDirty();
			}

			GenSlotNames();
			ImGui::TableNextColumn();
			ImGui::Combo("##SelectSlot", &_active_slot, &DescIO::StringVectorComboGetter, &_slot_names, _slot_names.size());

			desc.Visit(_active_slot, [this](auto& d) { Draw(d); });
			DescIO::EndForm();
		}
	}
	
	void TextureDocument::Draw(RasterTextureDesc& desc)
	{
		Draw(desc.base);
		DescIO::FormSeparator(this, "Storage");
		DRAW_FIELDS(RASTER_TEXTURE_PARTIAL_GENERATOR);
	}
	
	void TextureDocument::Draw(VectorTextureDesc& desc)
	{
		Draw(desc.base);
		DescIO::FormSeparator(this, "Storage");
		DRAW_FIELDS(VECTOR_TEXTURE_PARTIAL_GENERATOR);
	}
	
	void TextureDocument::Draw(BaseTextureDesc& desc)
	{
		DescIO::FormSeparator(this, "Parameters");
		DRAW_FIELDS(TEXTURE_PARAMS_GENERATOR);

		DescIO::FormSeparator(this, "Animation");
		ImGui::BeginDisabled(_gif);
		DRAW_FIELD(anim);
		ImGui::EndDisabled();

		if (desc.anim.scratch && !_gif)
			Draw(desc.spritesheet);
	}

	void TextureDocument::Draw(SpritesheetDesc& desc)
	{
		DRAW_FIELD(rows);
		DRAW_FIELD(cols);
		
		DRAW_FIELD(enable_cell_width_override);
		ImGui::SameLine();
		ImGui::BeginDisabled(!desc.enable_cell_width_override.scratch);
		DRAW_FIELD(cell_width_override);
		ImGui::EndDisabled();
		
		DRAW_FIELD(enable_cell_height_override);
		ImGui::SameLine();
		ImGui::BeginDisabled(!desc.enable_cell_height_override.scratch);
		DRAW_FIELD(cell_height_override);
		ImGui::EndDisabled();

		DRAW_FIELD(delay_cs);
		DRAW_FIELD(row_major);
		DRAW_FIELD(row_up);
	}

	void TextureDocument::Load(TOMLNode node, TextureDescVariant& desc)
	{
		const auto Clear = [this](TextureDescVariant& desc) {
			if (_svg)
				desc.Clear<VectorTextureDesc>();
			else
				desc.Clear<RasterTextureDesc>();
		};

		TOMLArray array = node[detail::encode_key(detail::Key::TextureArray)].as_array();
		if (array && !array->empty())
		{
			Clear(desc);
			for (size_t i = 0; i < array->size(); ++i)
				desc.PushBack();

			desc.VisitIndexed([this, &array](size_t i, auto& d) { Load(TOMLNode(*array->get(i)), d); });
		}
		else
		{
			Clear(desc);
			desc.PushBack();

			desc.Visit(0, [this](auto& d) { Load(TOMLNode(), d); });
		}
	}
	
	void TextureDocument::Load(TOMLNode node, RasterTextureDesc& desc)
	{
		Load(node, desc.base);
		LOAD_FIELDS(RASTER_TEXTURE_PARTIAL_GENERATOR);
	}
	
	void TextureDocument::Load(TOMLNode node, VectorTextureDesc& desc)
	{
		Load(node, desc.base);
		LOAD_FIELDS(VECTOR_TEXTURE_PARTIAL_GENERATOR);
	}
	
	void TextureDocument::Load(TOMLNode node, BaseTextureDesc& desc)
	{
		LOAD_FIELDS(TEXTURE_PARAMS_GENERATOR);

		if (_gif)
			desc.anim.scratch = true;
		else
		{
			LOAD_FIELD(anim);
			Load(node, desc.spritesheet);
		}
	}

	void TextureDocument::Load(TOMLNode node, SpritesheetDesc& desc)
	{
		LOAD_FIELDS(SPRITESHEET_GENERATOR);
	}

	void TextureDocument::Dump(toml::table& table, TextureDescVariant& desc)
	{
		toml::v3::array array;
		desc.Visit([this, &array](auto& d) {
			toml::table table;
			Dump(table, d);
			array.push_back(std::move(table));
		});
		table.insert_or_assign(detail::encode_key(detail::Key::TextureArray), std::move(array));
	}

	void TextureDocument::Dump(toml::table& table, RasterTextureDesc& desc)
	{
		Dump(table, desc.base);
		DUMP_FIELDS(RASTER_TEXTURE_PARTIAL_GENERATOR);
	}

	void TextureDocument::Dump(toml::table& table, VectorTextureDesc& desc)
	{
		Dump(table, desc.base);
		DUMP_FIELDS(VECTOR_TEXTURE_PARTIAL_GENERATOR);
	}

	void TextureDocument::Dump(toml::table& table, BaseTextureDesc& desc)
	{
		DUMP_FIELDS(TEXTURE_PARAMS_GENERATOR);
		DUMP_FIELD(anim);
		if (desc.anim.scratch && !_gif)
			Dump(table, desc.spritesheet);
	}

	void TextureDocument::Dump(toml::table& table, SpritesheetDesc& desc)
	{
		DUMP_FIELDS(SPRITESHEET_GENERATOR);
	}

	void TextureDocument::GenSlotNames()
	{
		_slot_names.clear();
		for (int i = 0; i < _scratch.Count(); ++i)
			_slot_names.push_back("Slot " + std::to_string(i));

		if (_scratch.Empty())
			_active_slot = 0;
		else if (_active_slot >= _scratch.Count())
			_active_slot = _scratch.Count() - 1;
	}
}
