#include "TextureDocument.h"

#include "definitions/Keys.h"

#include "core/MainWindow.h"
#include "core/Logger.h"

#include "core/ResourceLoader.h"
#include "graphics/Toolbar.h"

#include <imgui.h>
#include <imgui_internal.h>

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

	// TODO v8 asset editor panel draw menu bar in document to have document-specific bars. Here, add options to save, revert fully, etc.
	// TODO v8 revert button for slot count

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

			MarkDirty();
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

			SpritesheetDesc* spritesheet_desc = SpritesheetPreview();

			if (spritesheet_desc)
			{
				ImGui::SameLine();
				Toolbar::DrawIconToggleButton(Resource::PreviewIcon, _preview_spritesheet, "Preview spritesheet");
				ImGui::SameLine();
				Toolbar::DrawIconToggleButton(Resource::PauseIcon, Resource::PlayIcon, _spritesheet_preview_data.playing, "Play/pause animation");
				ImGui::SameLine();
				if (Toolbar::DrawIconButton(Resource::StopIcon, "Stop animation", "StopAnimation"))
					_spritesheet_preview_data = {};
			}
			else
				_spritesheet_preview_data = {};

			if (ImGui::IsWindowHovered())
			{
				_preview_nav.zoom += ImGui::GetIO().MouseWheel;

				if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
					_preview_nav.pos += ImGui::GetIO().MouseDelta;
			}

			if (_spritesheet_preview_data.playing && spritesheet_desc)
				PlaySpritesheetAnimation(*spritesheet_desc);
			else
			{
				ImVec2 avail = ImGui::GetContentRegionAvail();
				ImVec2 cursor = ImGui::GetCursorScreenPos();
				ImVec2 size = _texture.Size() * std::pow(2.f, _preview_nav.zoom);

				ImVec2 offset = 0.5f * (avail - size) + _preview_nav.pos;
				ImVec2 pos = cursor + offset;

				ImGui::GetWindowDrawList()->AddImage(_texture.ID(), pos, pos + size);
				if (_preview_spritesheet && spritesheet_desc)
					DrawSpritesheetOverlay(*spritesheet_desc, pos);
			}
			ImGui::EndChild();
		}
	}

	SpritesheetDesc* TextureDocument::SpritesheetPreview()
	{
		if (_gif)
			return nullptr;
		else if (auto d = _scratch.Visit(_active_slot, [](auto& desc) -> SpritesheetDesc* { return desc.base.anim.scratch ? &desc.base.spritesheet : nullptr; }))
			return *d;
		else
			return nullptr;
	}

	SpritesheetInfo TextureDocument::CalcSpritesheetInfo(const SpritesheetDesc& desc)
	{
		const ImVec2 texture_size = _texture.Size() * _preview_nav.svg_scale * std::pow(2.f, _preview_nav.zoom);

		int cols = desc.cols.scratch;
		float cell_width = desc.cell_width_override.scratch;

		if (desc.enable_cell_width_override.scratch)
			cols = static_cast<int>(texture_size.x) / static_cast<int>(cell_width);
		else
			cell_width = texture_size.x / cols;

		const float full_width = cols * cell_width;

		int rows = desc.rows.scratch;
		float cell_height = desc.cell_height_override.scratch;

		if (desc.enable_cell_height_override.scratch)
			rows = static_cast<int>(texture_size.y) / static_cast<int>(cell_height);
		else
			cell_height = texture_size.y / rows;

		const float full_height = rows * cell_height;

		return {
			.rows = rows,
			.cols = cols,
			.cell_width = cell_width,
			.cell_height = cell_height,
			.full_width = full_width,
			.full_height = full_height,
			.texture_width = texture_size.x,
			.texture_height = texture_size.y
		};
	}

	void TextureDocument::DrawSpritesheetOverlay(const SpritesheetDesc& desc, ImVec2 rect_start)
	{
		auto info = CalcSpritesheetInfo(desc);
		auto dl = ImGui::GetWindowDrawList();

		std::vector<int> xpos(info.cols + 1);

		for (int i = 0; i <= info.cols; ++i)
			xpos[i] = i * info.full_width / info.cols;

		for (int x : xpos)
			dl->AddLine(rect_start + ImVec2(x, 0), rect_start + ImVec2(x, info.full_height), IM_COL32_WHITE);

		std::vector<int> ypos(info.rows + 1);

		for (int i = 0; i <= info.rows; ++i)
			ypos[i] = i * info.full_height / info.rows;

		for (int y : ypos)
			dl->AddLine(rect_start + ImVec2(0, y), rect_start + ImVec2(info.full_width, y), IM_COL32_WHITE);

		const auto DrawDigit = [dl, rect_start, &xpos, &ypos](int x, int y, int digit) {
			const std::string d = std::to_string(digit);
			ImFont* font = ImGui::GetFont();
			const ImVec2 text_size = font->CalcTextSizeA(1.f, FLT_MAX, 0.f, d.c_str());

			const ImVec2 box_start = rect_start + ImVec2(xpos[x], ypos[y]);
			const ImVec2 box_end = rect_start + ImVec2(xpos[x + 1], ypos[y + 1]);
			const ImVec2 box_size = box_end - box_start;

			const float scale_x = box_size.x / text_size.x;
			const float scale_y = box_size.y / text_size.y;
			const float font_scale = (scale_x < scale_y) ? scale_x : scale_y;
			const float font_size = font_scale;

			if (ImGui::GetRoundedFontSize(font_size) > 0)
			{
				for (int dx = -1; dx <= 1; ++dx)
				{
					for (int dy = -1; dy <= 1; ++dy)
					{
						if (dx != 0 || dy != 0)
							dl->AddText(font, font_size, box_start + ImVec2(dx, dy) * 1.5f, IM_COL32_BLACK, d.c_str());
					}
				}

				dl->AddText(font, font_size, box_start, IM_COL32_WHITE, d.c_str());
			}
		};

		int digit = 0;
		if (desc.row_major.scratch)
		{
			if (desc.row_up.scratch)
			{
				for (int i = info.rows - 1; i >= 0; --i)
					for (int j = 0; j < info.cols; ++j)
						DrawDigit(j, i, digit++);
			}
			else
			{
				for (int i = 0; i < info.rows; ++i)
					for (int j = 0; j < info.cols; ++j)
						DrawDigit(j, i, digit++);
			}
		}
		else
		{
			if (desc.row_up.scratch)
			{
				for (int j = 0; j < info.cols; ++j)
					for (int i = info.rows - 1; i >= 0; --i)
						DrawDigit(j, i, digit++);
			}
			else
			{
				for (int j = 0; j < info.cols; ++j)
					for (int i = 0; i < info.rows; ++i)
						DrawDigit(j, i, digit++);
			}
		}
	}

	void TextureDocument::PlaySpritesheetAnimation(const SpritesheetDesc& desc)
	{
		auto info = CalcSpritesheetInfo(desc);

		_spritesheet_preview_data.timer += ImGui::GetIO().DeltaTime;
		if (desc.delay_cs.scratch > 0.f)
		{
			while (_spritesheet_preview_data.timer >= desc.delay_cs.scratch * 0.01f)
			{
				_spritesheet_preview_data.timer -= desc.delay_cs.scratch * 0.01f;
				++_spritesheet_preview_data.active_index;
			}
			_spritesheet_preview_data.active_index %= info.rows * info.cols;
		}
		else
		{
			_spritesheet_preview_data.timer = 0.f;
			_spritesheet_preview_data.active_index = 0;
		}

		ImVec2 avail = ImGui::GetContentRegionAvail();
		ImVec2 cursor = ImGui::GetCursorScreenPos();
		ImVec2 size = ImVec2(info.cell_width, info.cell_height) * std::pow(2.f, _preview_nav.zoom);

		ImVec2 offset = 0.5f * (avail - size) + _preview_nav.pos;
		ImVec2 pos = cursor + offset;

		const int active_index = _spritesheet_preview_data.active_index;
		const int row1 = desc.row_up.scratch ? info.rows - active_index / info.cols : active_index / info.cols;
		const int row2 = desc.row_up.scratch ? row1 - 1 : row1 + 1;
		const int col1 = desc.row_major.scratch ? active_index % info.cols : info.cols - (active_index % info.cols);
		const int col2 = desc.row_major.scratch ? col1 + 1 : col1 - 1;

		ImVec2 uv_min = ImVec2(std::min(col1, col2) * info.cell_width / info.texture_width, std::min(row1, row2) * info.cell_height / info.texture_height);
		ImVec2 uv_max = ImVec2(std::max(col1, col2) * info.cell_width / info.texture_width, std::max(row1, row2) * info.cell_height / info.texture_height);

		ImGui::GetWindowDrawList()->AddImage(_texture.ID(), pos, pos + size, uv_min, uv_max);
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
