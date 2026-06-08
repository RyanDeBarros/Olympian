#include "TextureDocument.h"

#include "definitions/Keys.h"

#include "core/MainWindow.h"
#include "core/Logger.h"

#include "core/ResourceLoader.h"
#include "gui/DisabledSection.h"
#include "gui/Subform.h"
#include "gui/Toolbar.h"
#include "gui/ImGuiWrapper.h"

#include <imgui_internal.h>

namespace oly::editor
{
	const char* TextureDocument::GetVersion()
	{
		return "1.0";
	}

	void TextureDocument::Init()
	{
		if (!GetSourcePath().is_resource())
		{
			Notification notif(LogLevel::Warning, "Asset is not located in resource folder");
			MainWindow::Instance().PushNotification(std::move(notif));
		}

		_gif = GetSourcePath().extension_matches(".gif");
		_svg = GetSourcePath().extension_matches(".svg");
		Load();
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

	void TextureDocument::DrawMenuBar()
	{
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

			MarkClean();
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

		_active_slot = 0;
		_preview_nav = {};
		if (auto svg_desc = std::get_if<ArrayDesc<VectorTextureDesc>>(&_scratch.variant))
			_preview_nav.svg_scale = svg_desc->array[_active_slot]->scale.scratch;

		ReloadPreviewTexture();
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

	void TextureDocument::ReloadPreviewTexture()
	{
		std::optional<GLenum> min_filter = _scratch.Visit(_active_slot, [](const auto& desc) -> GLenum { return desc.base.min_filter.Scratch(); });
		std::optional<GLenum> mag_filter = _scratch.Visit(_active_slot, [](const auto& desc) -> GLenum { return desc.base.mag_filter.Scratch(); });

		if (_svg)
			_texture = { SVGTexture(GetSourcePath().string().c_str(), _preview_nav.svg_scale, min_filter ? *min_filter : GL_LINEAR, mag_filter ? *mag_filter : GL_LINEAR) };
		else if (_gif)
			_texture = { GIFTexture(GetSourcePath().string().c_str(), min_filter ? *min_filter : GL_NEAREST, mag_filter ? *mag_filter : GL_NEAREST) };
		else
			_texture = { RasterTexture(GetSourcePath().string().c_str(), min_filter ? *min_filter : GL_NEAREST, mag_filter ? *mag_filter : GL_NEAREST) };
	}

	void TextureDocument::DrawPreview()
	{
		if (ImGui::BeginChild("Preview", ImVec2(0, 0), ImGuiChildFlags_Borders))
		{
			ImGui::Text("Preview");
			ImGui::Separator();
			if (Toolbar::DrawIconButton(IconResource::Recenter, "Reset panning/zoom", &_preview_nav))
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
				if (Toolbar::DrawIconButton(IconResource::Refresh, "Refresh SVG scale", &svg->preview_scale))
				{
					_preview_nav.svg_scale = scale;
					_texture = { SVGTexture(GetSourcePath().string().c_str(), _preview_nav.svg_scale) };
				}
			}

			SpritesheetDesc* spritesheet_desc = SpritesheetPreview();

			if (spritesheet_desc)
			{
				ImGui::SameLine();
				Toolbar::DrawIconToggleButton(IconResource::Preview, _preview_spritesheet, "Preview spritesheet");
				ImGui::SameLine();
				Toolbar::DrawIconToggleButton(IconResource::Pause, IconResource::Play, _spritesheet_preview_data.playing, "Play/pause animation");
				ImGui::SameLine();
				if (Toolbar::DrawIconButton(IconResource::Stop, "Stop animation", "StopAnimation"))
					_spritesheet_preview_data = {};
			}
			else
				_spritesheet_preview_data = {};

			if (ImGui::IsWindowHovered())
			{
				const int wheel = ImGui::GetIO().MouseWheel;
				if (wheel != 0)
				{
					float scale = std::pow(2.0f, wheel);

					ImVec2 avail = ImGui::GetContentRegionAvail();
					ImVec2 cursor = ImGui::GetCursorScreenPos();
					ImVec2 mouse = ImGui::GetIO().MousePos;
					ImVec2 center = cursor + 0.5f * avail;
					ImVec2 mouse_offset = mouse - center;
					_preview_nav.pos = mouse_offset + scale * (_preview_nav.pos - mouse_offset);
					_preview_nav.zoom += wheel;
				}
			}

			ImVec2 pos = ImGui::GetCursorScreenPos();
			ImGui::InvisibleButton("PreviewCanvas", ImGui::GetContentRegionAvail(), ImGuiButtonFlags_MouseButtonLeft);
			if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
				_preview_nav.pos += ImGui::GetIO().MouseDelta;
			ImGui::SetCursorScreenPos(pos);

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
					DrawSpritesheetOverlay(*spritesheet_desc, pos, size);
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
		int xoff = _texture.Width() > 1 ? std::min(desc.col_offset_pixel.scratch, static_cast<int>(_texture.Width())) : 0;
		int working_width = static_cast<int>(_texture.Width()) - xoff;

		int cols = desc.col_type.scratch == detail::SpritesheetParamType::Index ? desc.col_value.scratch : 1;
		float cell_width = desc.col_type.scratch == detail::SpritesheetParamType::Pixel ? desc.col_value.scratch : 1;

		if (desc.col_type.scratch == detail::SpritesheetParamType::Index)
			cell_width = static_cast<float>(working_width) / cols;
		else
			cols = working_width / static_cast<int>(cell_width);

		int col_offset = std::min(desc.col_offset_index.scratch, cols);
		cols -= col_offset;

		const float full_width = cols * cell_width;

		int yoff = _texture.Height() > 1 ? std::min(desc.row_offset_pixel.scratch, static_cast<int>(_texture.Height())) : 0;
		int working_height = static_cast<int>(_texture.Height()) - yoff;

		int rows = desc.row_type.scratch == detail::SpritesheetParamType::Index ? desc.row_value.scratch : 1;
		float cell_height = desc.row_type.scratch == detail::SpritesheetParamType::Pixel ? desc.row_value.scratch : 1;

		if (desc.row_type.scratch == detail::SpritesheetParamType::Index)
			cell_height = static_cast<float>(working_height) / rows;
		else
			rows = working_height / static_cast<int>(cell_height);

		int row_offset = std::min(desc.row_offset_index.scratch, rows);
		rows -= row_offset;

		const float full_height = rows * cell_height;
		return {
			.rows = rows,
			.cols = cols,
			.cell_width = cell_width,
			.cell_height = cell_height,
			.full_width = full_width,
			.full_height = full_height,
			.rect_offset = ImVec2(xoff + col_offset * cell_width, yoff + row_offset * cell_height)
		};
	}

	void TextureDocument::DrawSpritesheetOverlay(const SpritesheetDesc& desc, ImVec2 rect_start, ImVec2 size)
	{
		auto info = CalcSpritesheetInfo(desc);
		auto dl = ImGui::GetWindowDrawList();

		ImVec2 scale = size / _texture.Size();
		rect_start += info.rect_offset * scale;

		std::vector<int> xpos(info.cols + 1);

		for (int i = 0; i <= info.cols; ++i)
			xpos[i] = i * info.full_width / info.cols;

		for (int x : xpos)
			dl->AddLine(rect_start + ImVec2(x, 0) * scale, rect_start + ImVec2(x, info.full_height) * scale, IM_COL32_WHITE);

		std::vector<int> ypos(info.rows + 1);

		for (int i = 0; i <= info.rows; ++i)
			ypos[i] = i * info.full_height / info.rows;

		for (int y : ypos)
			dl->AddLine(rect_start + ImVec2(0, y) * scale, rect_start + ImVec2(info.full_width, y) * scale, IM_COL32_WHITE);

		const auto DrawDigit = [dl, rect_start, &xpos, &ypos, scale](int x, int y, int digit) {
			const std::string d = std::to_string(digit);
			ImFont* font = ImGui::GetFont();
			const ImVec2 text_size = font->CalcTextSizeA(1.f, FLT_MAX, 0.f, d.c_str());

			const ImVec2 box_start = rect_start + ImVec2(xpos[x], ypos[y]) * scale;
			const ImVec2 box_end = rect_start + ImVec2(xpos[x + 1], ypos[y + 1]) * scale;
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
		if (desc.delay.scratch > 0.f)
		{
			while (_spritesheet_preview_data.timer >= desc.delay.scratch)
			{
				_spritesheet_preview_data.timer -= desc.delay.scratch;
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

		ImVec2 uv_min = ImVec2(std::min(col1, col2) * info.cell_width / _texture.Width(), std::min(row1, row2) * info.cell_height / _texture.Height());
		ImVec2 uv_max = ImVec2(std::max(col1, col2) * info.cell_width / _texture.Width(), std::max(row1, row2) * info.cell_height / _texture.Height());

		ImVec2 uv_offset = info.rect_offset / _texture.Size();
		uv_min += uv_offset;
		uv_max += uv_offset;

		ImGui::GetWindowDrawList()->AddImage(_texture.ID(), pos, pos + size, uv_min, uv_max);
	}

	void TextureDocument::Draw(TextureVariantDesc& desc)
	{
		if (auto form = Form())
		{
			bool slot_changed = false;
			const int og_slot = _active_slot;

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Text("Select Slot");

			GenSlotNames();
			ImGui::TableNextColumn();
			gui::Combo("##SelectSlot", _active_slot, _slot_names);

			ImGui::SameLine();
			static const unsigned char PLUS_ID = 0;
			if (Toolbar::DrawIconButton(IconResource::Plus, "New texture slot", &PLUS_ID))
			{
				_active_slot = _slot_names.size();
				desc.PushBack();
				MarkDirty();
				slot_changed = true;
			}

			ImGui::SameLine();
			static const unsigned char MINUS_ID = 0;
			if (Toolbar::DrawIconButton(IconResource::Minus, "Remove texture slot", &MINUS_ID))
			{
				desc.Remove(_active_slot);
				if (desc.Empty())
					desc.PushBack();
				MarkDirty();
				slot_changed = true;
			}

			if (desc.Size().disk && desc.Size().disk->scratch != desc.Size().scratch)
			{
				ImGui::SameLine();
				if (Toolbar::DrawIconButton(IconResource::Revert, "Revert", &desc.Size()))
				{
					desc.Resize(_disk);
					
					MarkDirty();
				}
			}

			if (!ClampActiveSlot(desc))
			{
				if (og_slot != _active_slot || slot_changed)
					ReloadPreviewTexture();
			}

			desc.Visit(_active_slot, [this, &form](auto& d) { Draw(form, d); });

		}
	}
	
	void TextureDocument::Draw(Form& form, RasterTextureDesc& desc)
	{
		Draw(form, desc.base);
		if (auto subform = Subform(form, "Storage", true))
		{
			DRAW_FIELDS(RASTER_TEXTURE_PARTIAL_GENERATOR);
		}
	}
	
	void TextureDocument::Draw(Form& form, VectorTextureDesc& desc)
	{
		Draw(form, desc.base);
		if (auto subform = Subform(form, "Storage", true))
		{
			DRAW_FIELDS(VECTOR_TEXTURE_PARTIAL_GENERATOR);
		}
	}
	
	void TextureDocument::Draw(Form& form, BaseTextureDesc& desc)
	{
		if (auto subform = Subform(form, "Parameters", true))
		{
			bool filter_changed = false;

			if (desc.min_filter.Draw())
			{
				MarkDirty();
				filter_changed = true;
			}

			if (desc.mag_filter.Draw())
			{
				MarkDirty();
				filter_changed = true;
			}

			if (filter_changed)
				ReloadPreviewTexture();

			DRAW_FIELD(wrap_s);
			DRAW_FIELD(wrap_t);
		}

		if (auto subform = Subform(form, "Animation", true))
		{
			if (auto disabled = DisabledSection(_gif))
			{
				DRAW_FIELD(anim);
			}

			if (desc.anim.scratch && !_gif)
				Draw(form, desc.spritesheet);
		}
	}

	void TextureDocument::Draw(Form& form, SpritesheetDesc& desc)
	{
		DRAW_FIELD(col_type);
		const char* col_label = desc.col_type.scratch == detail::SpritesheetParamType::Index ? "# Columns" : "Cell Width";
		if (DescIO::Draw(col_label, desc.col_value.scratch, DISK_FIELD(desc.col_value.disk), desc.col_value.Min, desc.col_value.Max))
			MarkDirty();

		DRAW_FIELD(row_type);
		const char* row_label = desc.row_type.scratch == detail::SpritesheetParamType::Index ? "# Rows" : "Cell Height";
		if (DescIO::Draw(row_label, desc.row_value.scratch, DISK_FIELD(desc.row_value.disk), desc.row_value.Min, desc.row_value.Max))
			MarkDirty();

		DRAW_FIELDS(SPRITESHEET_PARTIAL_GENERATOR);
	}

	void TextureDocument::Load(TOMLNode node, TextureVariantDesc& desc)
	{
		const auto Clear = [this](TextureVariantDesc& desc) {
			if (_svg)
				desc.Clear<VectorTextureDesc>();
			else
				desc.Clear<RasterTextureDesc>();
		};

		TOMLArray array = node[detail::encode_key(desc.array_key)].as_array();
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

		desc.Size().scratch = desc.Count();
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

	void TextureDocument::Dump(toml::table& table, TextureVariantDesc& desc)
	{
		toml::v3::array array;
		desc.Visit([this, &array](auto& d) {
			toml::table table;
			Dump(table, d);
			array.push_back(std::move(table));
		});
		table.insert_or_assign(detail::encode_key(desc.array_key), std::move(array));
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
		ClampActiveSlot(_scratch);
	}

	bool TextureDocument::ClampActiveSlot(TextureVariantDesc& desc)
	{
		const int og = _active_slot;

		if (desc.Empty())
			_active_slot = 0;
		else if (_active_slot >= desc.Count())
			_active_slot = desc.Count() - 1;

		if (og != _active_slot)
		{
			OnActiveSlotChanged();
			return true;
		}
		else
			return false;
	}

	void TextureDocument::OnActiveSlotChanged()
	{
		ReloadPreviewTexture();
	}
}
