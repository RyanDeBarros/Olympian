#include "TextureDocument.h"

#include "core/editor/ResourceLoader.h"

#include "assets/TranslateKey.h"
#include "definitions/Keys.h"

#include <imgui_internal.h>

namespace oly::editor
{
	SpritesheetPreviewData::SpritesheetPreviewData()
	{
		preview.config.icon = Icon(IconResource::Preview);
		preview.config.str_id = "##Preview";
		preview.config.selected = true;
		preview.config.tooltip = "Preview spritesheet";

		playing.config.icon = Icon(IconResource::Pause);
		playing.config.str_id = "##Playing";
		playing.config.selected_icon = Icon(IconResource::Play);
		playing.config.selected = false;
		playing.config.tooltip = "Play/pause animation";
	}

	const char* TextureDocument::GetVersion()
	{
		return "1.0";
	}

	void TextureDocument::InitImpl()
	{
		if (!GetSourcePath().is_resource())
			imtk::notify_warning("Asset is not located in resource folder");

		_gif = GetSourcePath().extension_matches(".gif");
		_svg = GetSourcePath().extension_matches(".svg");
		_slots.policy = gui::ListPolicy::MinimumOne;

		LoadAsset();
	}

	void TextureDocument::Draw()
	{
		auto pre_draw = PreDraw();

		UpdatePreviewTexture();

		imtk::id_scope scope(this);
		if (auto _ = imtk::table("", 2))
		{
			ImGui::TableNextColumn();
			Draw(_desc.scratch);

			ImGui::TableNextColumn();
			DrawPreview();
		}
	}

	void TextureDocument::LoadImpl()
	{
		if (_oly_path.is_file())
		{
			_meta = detail::MetaSplitter::decode_meta(_oly_path);

			toml::table table;
			std::string err = _oly_path.load_toml(table);
			if (err.empty())
				Load(imtk::toml_node(table), _desc.disk, _svg, _gif);
			else
				imtk::notify_error("cannot load texture - corrupted asset: " + GetSourcePath().string());

			MarkClean();
		}
		else
		{
			Load(imtk::toml_node(), _desc.disk, _svg, _gif);

			_meta = {};
			_meta.map[detail::Key::Meta_Version] = GetVersion();
			_meta.map[detail::Key::Meta_Import] = "1";
			_meta.map[detail::Key::Meta_Type] = detail::encode_key(detail::Key::Meta_Texture);

			MarkDirty();
		}

		_desc.LoadFromDisk();

		_slots.Init(*ListAdapter());

		_preview_nav = {};
		if (auto svg_desc = _desc.scratch.variant.try_get<imtk::desc::vector<VectorTextureDesc>>())
			_preview_nav.svg_scale = (*svg_desc)[_slots.active_index].scale.value;

		_stale_preview_texture = true;
	}

	void TextureDocument::DumpImpl()
	{
		toml::table table;
		Dump(table, _desc.scratch);
		_oly_path.dump_toml(table, _meta);
		_desc.WriteToDisk();
		MarkClean();
	}

	void TextureDocument::ResetAssetImpl()
	{
		Load(imtk::toml_node(), _desc.scratch, _svg, _gif);
	}

	const IDoubleDescriptor& TextureDocument::GetDoubleDescriptor() const
	{
		return _desc;
	}

	IDoubleDescriptor& TextureDocument::GetDoubleDescriptor()
	{
		return _desc;
	}

	detail::ResourcePath TextureDocument::GetSourcePath() const
	{
		return _oly_path.get_source_path();
	}

	void TextureDocument::UpdatePreviewTexture()
	{
		if (!_stale_preview_texture)
			return;

		_stale_preview_texture = false;

		std::optional<GLenum> min_filter = _desc.scratch.Visit(_slots.active_index, [](const auto& desc) -> GLenum { return desc.base.min_filter.Value(); });
		std::optional<GLenum> mag_filter = _desc.scratch.Visit(_slots.active_index, [](const auto& desc) -> GLenum { return desc.base.mag_filter.Value(); });
		std::optional<bool> generate_mipmaps = _desc.scratch.Visit(_slots.active_index, [](const auto& desc) -> bool {
			if constexpr (std::is_same_v<decltype(desc.generate_mipmaps.value), bool>)
				return desc.generate_mipmaps.value;
			else
				return desc.generate_mipmaps.value != detail::SVGMipmapGenerationMode::Off;
		});

		if (_svg)
			_texture = { imtk::svg_texture::load(GetSourcePath().string().c_str(), _preview_nav.svg_scale, min_filter, mag_filter, generate_mipmaps ? *generate_mipmaps : false) };
		else if (_gif)
			_texture = { imtk::gif_texture::load(GetSourcePath().string().c_str(), min_filter, mag_filter, generate_mipmaps ? *generate_mipmaps : false) };
		else
			_texture = { imtk::raster_texture::load(GetSourcePath().string().c_str(), min_filter, mag_filter, generate_mipmaps ? *generate_mipmaps : false) };
	}

	void TextureDocument::DrawPreview()
	{
		if (auto _ = imtk::child("Preview", ImVec2(0, 0), ImGuiChildFlags_Borders))
		{
			ImGui::TextUnformatted("Preview");
			ImGui::Separator();

			if (imtk::w::icon_button({ .icon = Icon(IconResource::Recenter), .str_id = "##Recenter", .tooltip = "Reset panning/zoom" }).draw())
			{
				_preview_nav = {};
				if (imtk::svg_texture* svg = _texture.get_svg())
					_texture = { imtk::svg_texture::load(GetSourcePath().string().c_str(), _preview_nav.svg_scale) };
			}
			
			if (imtk::gif_texture* gif = _texture.get_gif())
			{
				imtk::controls::vertical_separator();
				ImGui::SetNextItemWidth(100.0f);
				ImGui::InputFloat("Speed", &gif->speed);
				gif->update();
			}

			if (imtk::svg_texture* svg = _texture.get_svg())
			{
				imtk::controls::vertical_separator();
				ImGui::SetNextItemWidth(100.0f);
				float scale = svg->preview_scale * _preview_nav.svg_scale;
				ImGui::InputFloat("Scale", &scale);
				svg->preview_scale = scale / _preview_nav.svg_scale;
				imtk::controls::vertical_separator();
				if (imtk::w::icon_button({ .icon = Icon(IconResource::Refresh), .str_id = "##RefreshSVGScale", .tooltip = "Refresh SVG scale"}).draw())
				{
					_preview_nav.svg_scale = scale;
					_texture = { imtk::svg_texture::load(GetSourcePath().string().c_str(), _preview_nav.svg_scale) };
				}
			}

			SpritesheetDesc* spritesheet_desc = SpritesheetPreview();

			if (spritesheet_desc)
			{
				imtk::controls::vertical_separator();
				_spritesheet_preview_data.preview.draw();
				ImGui::SameLine();
				_spritesheet_preview_data.playing.draw();
				ImGui::SameLine();
				if (imtk::w::icon_button({ .icon = Icon(IconResource::Stop), .str_id = "##StopAnimation", .tooltip = "Stop animation"}).draw())
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

			if (_spritesheet_preview_data.playing.selected() && spritesheet_desc)
				PlaySpritesheetAnimation(*spritesheet_desc);
			else
			{
				ImVec2 avail = ImGui::GetContentRegionAvail();
				ImVec2 cursor = ImGui::GetCursorScreenPos();
				ImVec2 size = ImVec2(_texture.width(), _texture.height()) * std::pow(2.f, _preview_nav.zoom);

				ImVec2 offset = 0.5f * (avail - size) + _preview_nav.pos;
				ImVec2 pos = cursor + offset;

				ImGui::GetWindowDrawList()->AddImage(_texture.id(), pos, pos + size);
				if (_spritesheet_preview_data.preview.selected() && spritesheet_desc)
					DrawSpritesheetOverlay(*spritesheet_desc, pos, size);
			}
		}
	}

	SpritesheetDesc* TextureDocument::SpritesheetPreview()
	{
		if (_gif)
			return nullptr;
		else if (auto d = _desc.scratch.Visit(_slots.active_index, [](auto& desc) -> SpritesheetDesc* { return desc.base.anim.value ? &desc.base.spritesheet : nullptr; }))
			return *d;
		else
			return nullptr;
	}

	SpritesheetInfo TextureDocument::CalcSpritesheetInfo(const SpritesheetDesc& desc)
	{
		int xoff = _texture.width() > 1 ? std::min(desc.col_offset_pixel.value, static_cast<int>(_texture.width())) : 0;
		int working_width = static_cast<int>(_texture.width()) - xoff;

		int cols = desc.col_type.value == detail::SpritesheetParamType::Index ? desc.col_value.value : 1;
		float cell_width = desc.col_type.value == detail::SpritesheetParamType::Pixel ? desc.col_value.value : 1;

		if (desc.col_type.value == detail::SpritesheetParamType::Index)
			cell_width = static_cast<float>(working_width) / cols;
		else
			cols = working_width / static_cast<int>(cell_width);

		int col_offset = std::min(desc.col_offset_index.value, cols);
		cols -= col_offset;

		const float full_width = cols * cell_width;

		int yoff = _texture.height() > 1 ? std::min(desc.row_offset_pixel.value, static_cast<int>(_texture.height())) : 0;
		int working_height = static_cast<int>(_texture.height()) - yoff;

		int rows = desc.row_type.value == detail::SpritesheetParamType::Index ? desc.row_value.value : 1;
		float cell_height = desc.row_type.value == detail::SpritesheetParamType::Pixel ? desc.row_value.value : 1;

		if (desc.row_type.value == detail::SpritesheetParamType::Index)
			cell_height = static_cast<float>(working_height) / rows;
		else
			rows = working_height / static_cast<int>(cell_height);

		int row_offset = std::min(desc.row_offset_index.value, rows);
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

		ImVec2 scale = ImVec2(size.x / _texture.width(), size.y / _texture.height());
		rect_start += info.rect_offset * scale;

		std::vector<int> xpos(info.cols + 1);

		for (int i = 0; i <= info.cols; ++i)
			xpos[i] = i * info.full_width / info.cols;

		for (int x : xpos)
			dl->AddLine(rect_start + ImVec2(x, 0) * scale, rect_start + ImVec2(x, info.full_height) * scale, imtk::col::white);

		std::vector<int> ypos(info.rows + 1);

		for (int i = 0; i <= info.rows; ++i)
			ypos[i] = i * info.full_height / info.rows;

		for (int y : ypos)
			dl->AddLine(rect_start + ImVec2(0, y) * scale, rect_start + ImVec2(info.full_width, y) * scale, imtk::col::white);

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
							dl->AddText(font, font_size, box_start + ImVec2(dx, dy) * 1.5f, imtk::col::black, d.c_str());
					}
				}

				dl->AddText(font, font_size, box_start, imtk::col::white, d.c_str());
			}
		};

		int digit = 0;
		if (desc.row_major.value)
		{
			if (desc.row_up.value)
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
			if (desc.row_up.value)
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
		if (desc.delay.value > 0.f)
		{
			while (_spritesheet_preview_data.timer >= desc.delay.value)
			{
				_spritesheet_preview_data.timer -= desc.delay.value;
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
		const int row1 = desc.row_up.value ? info.rows - active_index / info.cols : active_index / info.cols;
		const int row2 = desc.row_up.value ? row1 - 1 : row1 + 1;
		const int col1 = desc.row_major.value ? active_index % info.cols : info.cols - (active_index % info.cols);
		const int col2 = desc.row_major.value ? col1 + 1 : col1 - 1;

		ImVec2 uv_min = ImVec2(std::min(col1, col2) * info.cell_width / _texture.width(), std::min(row1, row2) * info.cell_height / _texture.height());
		ImVec2 uv_max = ImVec2(std::max(col1, col2) * info.cell_width / _texture.width(), std::max(row1, row2) * info.cell_height / _texture.height());

		ImVec2 uv_offset = ImVec2(info.rect_offset.x / _texture.width(), info.rect_offset.y / _texture.height());
		uv_min += uv_offset;
		uv_max += uv_offset;

		ImGui::GetWindowDrawList()->AddImage(_texture.id(), pos, pos + size, uv_min, uv_max);
	}

	void TextureDocument::Draw(TextureVariantDesc& desc)
	{
		_slots.Update(*ListAdapter());
		
		if (auto scope = imtk::id_scope("##Slot"))
			_slots.DrawComboHeader({ .prompt = "Select slot", .create_tooltip = "New texture slot", .delete_tooltip = "Delete texture slot", .clear_tooltip = "Clear texture slots" }, "Slot");

		if (auto form = imtk::prop::form())
		{
			desc.variant.visit([this](auto& desc_list) { Draw(desc_list[_slots.active_index]); });

			if (_slots.ConsumeOps(*ListAdapter()))
				MarkDirty();

			if (_slots.active_index.consume_modified())
				_stale_preview_texture = true;
		}
	}
	
	void TextureDocument::Draw(RasterTextureDesc& desc)
	{
		Draw(desc.base);
		if (auto subform = imtk::prop::subform("Storage", { .start_open = true }))
		{
			desc.generate_mipmaps.draw();
			if (imtk::prop::row::dirty())
				_stale_preview_texture = true;

			desc.storage.draw();
		}
	}
	
	void TextureDocument::Draw(VectorTextureDesc& desc)
	{
		Draw(desc.base);
		if (auto subform = imtk::prop::subform("Storage", { .start_open = true }))
		{
			desc.generate_mipmaps.draw();
			if (imtk::prop::row::dirty())
				_stale_preview_texture = true;

			desc.image_storage.draw();
			desc.abstract_storage.draw();
			desc.scale.draw();
		}
	}
	
	void TextureDocument::Draw(BaseTextureDesc& desc)
	{
		if (auto subform = imtk::prop::subform("Parameters", { .start_open = true }))
		{
			desc.min_filter.draw();
			if (imtk::prop::row::dirty())
				_stale_preview_texture = true;

			desc.mag_filter.draw();
			if (imtk::prop::row::dirty())
				_stale_preview_texture = true;

			desc.wrap_s.draw();
			desc.wrap_t.draw();
		}

		if (auto subform = imtk::prop::subform("Animation", { .start_open = true }))
		{
			if (auto d = imtk::disabled(_gif))
			{
				desc.anim.draw();
				if (imtk::prop::row::get_draw_result().state.hovered())
					ImGui::SetTooltip("Animation is always enabled for GIF textures");
			}

			if (desc.anim.value && !_gif)
				Draw(desc.spritesheet);
		}
	}

	void TextureDocument::Draw(SpritesheetDesc& desc)
	{
		desc.col_type.draw();
		desc.col_value.label = desc.col_type.value == detail::SpritesheetParamType::Index ? "# Columns" : "Cell Width";
		desc.col_value.draw();

		desc.row_type.draw();
		desc.row_value.label = desc.row_type.value == detail::SpritesheetParamType::Index ? "# Rows" : "Cell Height";
		desc.row_value.draw();

		IMTK_DRAW_FIELDS(SPRITESHEET_PARTIAL_GENERATOR);
	}

	void TextureDocument::Load(imtk::toml_node node, TextureVariantDesc& desc, bool svg, bool gif)
	{
		if (svg)
			desc.variant.set<imtk::desc::vector<VectorTextureDesc>>();
		else
			desc.variant.set<imtk::desc::vector<RasterTextureDesc>>();

		const toml::array* array = desc.variant.subnode(node).as_array();
		if (array && !array->empty())
		{
			for (size_t i = 0; i < array->size(); ++i)
				desc.PushBack();

			desc.VisitIndexed([&array, gif](size_t i, auto& d) { Load(imtk::toml_node(*array->get(i)), d, gif); });
		}
		else
		{
			desc.PushBack();

			desc.Visit(0, [gif](auto& d) { Load(imtk::toml_node(), d, gif); });
		}
	}
	
	void TextureDocument::Load(imtk::toml_node node, RasterTextureDesc& desc, bool gif)
	{
		Load(node, desc.base, gif);
		IMTK_LOAD_FIELDS(RASTER_TEXTURE_PARTIAL_GENERATOR);
	}
	
	void TextureDocument::Load(imtk::toml_node node, VectorTextureDesc& desc, bool gif)
	{
		Load(node, desc.base, gif);
		IMTK_LOAD_FIELDS(VECTOR_TEXTURE_PARTIAL_GENERATOR);
	}
	
	void TextureDocument::Load(imtk::toml_node node, BaseTextureDesc& desc, bool gif)
	{
		IMTK_LOAD_FIELDS(TEXTURE_PARAMS_GENERATOR);

		if (gif)
		{
			desc.anim.def = true;
			desc.anim.value = true;
		}
		else
		{
			desc.anim.load(node);
			Load(node, desc.spritesheet);
		}
	}

	void TextureDocument::Load(imtk::toml_node node, SpritesheetDesc& desc)
	{
		IMTK_LOAD_FIELDS(SPRITESHEET_GENERATOR);
	}

	void TextureDocument::Dump(toml::table& table, TextureVariantDesc& desc)
	{
		toml::array array;
		desc.variant.visit([this, &array](auto& d) {
			for (auto& desc : d)
				Dump(array.emplace_back<toml::table>(), desc);
		});
		desc.variant.dump_into(table, std::move(array));
	}

	void TextureDocument::Dump(toml::table& table, RasterTextureDesc& desc)
	{
		Dump(table, desc.base);
		IMTK_DUMP_FIELDS(RASTER_TEXTURE_PARTIAL_GENERATOR);
	}

	void TextureDocument::Dump(toml::table& table, VectorTextureDesc& desc)
	{
		Dump(table, desc.base);
		IMTK_DUMP_FIELDS(VECTOR_TEXTURE_PARTIAL_GENERATOR);
	}

	void TextureDocument::Dump(toml::table& table, BaseTextureDesc& desc)
	{
		IMTK_DUMP_FIELDS(TEXTURE_PARAMS_GENERATOR);
		desc.anim.dump(table);
		if (desc.anim.value && !_gif)
			Dump(table, desc.spritesheet);
	}

	void TextureDocument::Dump(toml::table& table, SpritesheetDesc& desc)
	{
		IMTK_DUMP_FIELDS(SPRITESHEET_GENERATOR);
	}

	void TextureDocument::OnActiveSlotChanged()
	{
		_stale_preview_texture = true;
	}

	struct BriefDescPrinter
	{
		void operator()(std::ostream& os, const RasterTextureDesc& desc) const
		{
			os << "RasterTextureDesc[...]";
		}

		void operator()(std::ostream& os, const VectorTextureDesc& desc) const
		{
			os << "VectorTextureDesc[...]";
		}
	};

	std::unique_ptr<gui::IListAdapter> TextureDocument::ListAdapter()
	{
		return _desc.scratch.variant.visit([this](auto& desc) { return gui::MakeVectorAdapter<BriefDescPrinter>(desc); });
	}

	TextureDocument::TextureSettingsLoadResult TextureDocument::LoadTextureSettings(const detail::ResourcePath path, int slot, GLenum& min_filter, GLenum& mag_filter, float& scale, bool& generate_mipmaps)
	{
		if (!path.is_file())
			return TextureSettingsLoadResult::NotAFile;

		auto oly_path = path.get_import_path();
		if (!oly_path.is_file())
			return TextureSettingsLoadResult::MissingImport;

		if (slot < 0)
			return TextureSettingsLoadResult::BadSlot;

		if (!detail::MetaSplitter::decode_meta(oly_path).has_type(detail::Key::Meta_Texture))
			return TextureSettingsLoadResult::NotATexture;

		toml::table table;
		std::string err = oly_path.load_toml(table);
		if (err.empty())
		{
			imtk::toml_node node = imtk::toml_node(table);

			TextureVariantDesc desc;

			const toml::array* array = desc.variant.subnode(node).as_array();
			if (!array || slot >= array->size() || !array->get(slot))
				return TextureSettingsLoadResult::BadSlot;

			bool gif = path.extension_matches(".gif");
			bool svg = path.extension_matches(".svg");
			Load(node, desc, svg, gif);
			
			desc.Visit(slot, [&](const auto& d) {
				min_filter = d.base.min_filter.Value();
				mag_filter = d.base.mag_filter.Value();

				if constexpr (std::is_same_v<std::decay_t<decltype(d)>, VectorTextureDesc>)
				{
					scale = d.scale.value;
					generate_mipmaps = d.generate_mipmaps.value != detail::SVGMipmapGenerationMode::Off;
				}
				else
					generate_mipmaps = d.generate_mipmaps.value;
			});
			
			if (!path.is_resource())
				return TextureSettingsLoadResult::NotAResource;

			return TextureSettingsLoadResult::Success;
		}
		else
			return TextureSettingsLoadResult::Corrupted;
	}
}
