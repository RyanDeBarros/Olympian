#include "TextureDocument.h"

#include "desc/DescIO.h"

#include "assets/TranslateKey.h"
#include "definitions/Keys.h"

#include <imgui.h>

namespace oly::editor
{
	// TODO v7 preview of image on right side of table

	void TextureDocument::Init()
	{
		_gif = GetSourcePath().extension_matches(".gif");
		_svg = GetSourcePath().extension_matches(".svg");
		Load();
	}

	void TextureDocument::Draw()
	{
		Draw(_desc, &_disk);
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
				// TODO v7 log error - corrupted asset
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
		_desc = _disk;
	}

	void TextureDocument::Dump()
	{
		toml::table table;
		Dump(table, _desc);
		_oly_path.dump_toml(table, _meta);
		_disk = _desc;
		MarkClean();
	}

	detail::ResourcePath TextureDocument::GetSourcePath() const
	{
		return _oly_path.get_source_path();
	}

	// TODO v7 combine args into structs for int fields, bool fields, etc. for clarity - that goes for all the DescIO methods too.

#define PROP_RASTER_TEXTURE(M) \
		M(storage, detail::Key::Storage, detail::StorageMode::Discard, "Storage"); \
		M(generate_mipmaps, detail::Key::GenerateMipmaps, false, "Generate Mipmaps");

#define PROP_VECTOR_TEXTURE(M) \
		M(scale, detail::Key::VectorScale, 1.f, "Vector Scale", 0.f, std::nullopt); \
		M(image_storage, detail::Key::ImageStorage, detail::StorageMode::Discard, "Image Storage"); \
		M(abstract_storage, detail::Key::AbstractStorage, detail::StorageMode::Discard, "Abstract Storage"); \
		M(generate_mipmaps, detail::Key::GenerateMipmaps, detail::SVGMipmapGenerationMode::Off, "Generate Mipmaps");

#define PROP_TEXTURE_PARAMS(M) \
		M(min_filter, detail::Key::MinFilter, GL_NEAREST, "Min Filter", min_filter_values, min_filter_names, IM_ARRAYSIZE(min_filter_names)); \
		M(mag_filter, detail::Key::MagFilter, GL_NEAREST, "Mag Filter", mag_filter_values, mag_filter_names, IM_ARRAYSIZE(mag_filter_names)); \
		M(wrap_s, detail::Key::WrapS, GL_CLAMP_TO_EDGE, "Wrap (S)", wrap_values, wrap_names, IM_ARRAYSIZE(wrap_names)); \
		M(wrap_t, detail::Key::WrapT, GL_CLAMP_TO_EDGE, "Wrap (T)", wrap_values, wrap_names, IM_ARRAYSIZE(wrap_names));

#define PROP_SPRITESHEET(M) \
		M(rows, detail::Key::Rows, 1, "Rows", 1, std::nullopt); \
		M(cols, detail::Key::Columns, 1, "Columns", 1, std::nullopt); \
		M(cell_width_override, detail::Key::CellWidthOverride, 0, "Cell Width Override", 0, std::nullopt); \
		M(cell_height_override, detail::Key::CellHeightOverride, 0, "Cell Height Override", 0, std::nullopt); \
		M(delay_cs, detail::Key::DelayCS, 0, "Delay (CS)", 0, std::nullopt); \
		M(row_major, detail::Key::RowMajor, true, "Row Major"); \
		M(row_up, detail::Key::RowUp, true, "Row Up");

	void TextureDocument::Draw(TextureDesc& desc, const TextureDesc* disk)
	{
		// TODO v7 combo box to select slot, buttons to create new, delete.
		for (size_t i = 0; i < desc.array.size(); ++i)
		{
			Draw(desc.array[i], (disk && i < disk->array.size()) ? &disk->array[i] : nullptr);
		}
	}
	
	void TextureDocument::Draw(TextureSlotDesc& desc, const TextureSlotDesc* disk)
	{
		if (DescIO::BeginForm(&desc))
		{
			std::visit([this, disk](auto& d) { Draw(d, disk ? std::get_if<std::decay_t<decltype(d)>>(&disk->variant) : nullptr); }, desc.variant);
			DescIO::EndForm();
		}
	}
	
	void TextureDocument::Draw(RasterTextureDesc& desc, const RasterTextureDesc* disk)
	{
		Draw(desc.base, disk ? &disk->base : nullptr);
		PROP_RASTER_TEXTURE(OLY_EDITOR_DESC_IO_DRAW_PROP);
	}
	
	void TextureDocument::Draw(VectorTextureDesc& desc, const VectorTextureDesc* disk)
	{
		Draw(desc.base, disk ? &disk->base : nullptr);
		PROP_VECTOR_TEXTURE(OLY_EDITOR_DESC_IO_DRAW_PROP);
	}
	
	void TextureDocument::Draw(BaseTextureDesc& desc, const BaseTextureDesc* disk)
	{
		static const GLenum min_filter_values[] = {
			GL_NEAREST,
			GL_LINEAR,
			GL_NEAREST_MIPMAP_NEAREST,
			GL_LINEAR_MIPMAP_NEAREST,
			GL_NEAREST_MIPMAP_LINEAR,
			GL_LINEAR_MIPMAP_LINEAR
		};

		static const char* min_filter_names[] = {
			"Nearest",
			"Linear",
			"Nearest (Nearest Mipmap)",
			"Linear (Nearest Mipmap)",
			"Nearest (Linear Mipmap)",
			"Linear (Linear Mipmap)"
		};

		static const GLenum mag_filter_values[] = {
			GL_NEAREST,
			GL_LINEAR,
		};

		static const char* mag_filter_names[] = {
			"Nearest",
			"Linear",
		};

		static const GLenum wrap_values[] = {
			GL_CLAMP_TO_EDGE,
			GL_CLAMP_TO_BORDER,
			GL_MIRRORED_REPEAT,
			GL_REPEAT,
			GL_MIRROR_CLAMP_TO_EDGE
		};

		static const char* wrap_names[] = {
			"Clamp To Edge",
			"Clamp To Border",
			"Repeat (Mirrored)",
			"Repeat",
			"Clamp To Edge (Mirrored)"
		};

		PROP_TEXTURE_PARAMS(OLY_EDITOR_DESC_IO_DRAW_PROP);

		ImGui::BeginDisabled(_gif);
		OLY_EDITOR_DESC_IO_DRAW_FIELD("Animated", anim);
		ImGui::EndDisabled();

		if (desc.anim)
			Draw(desc.spritesheet, disk ? &disk->spritesheet : nullptr);
	}

	void TextureDocument::Draw(SpritesheetDesc& desc, const SpritesheetDesc* disk)
	{
		PROP_SPRITESHEET(OLY_EDITOR_DESC_IO_DRAW_PROP);
	}

	void TextureDocument::Load(TOMLNode node, TextureDesc& desc)
	{
		desc.array.clear();
		
		TOMLArray array = node[detail::encode_key(detail::Key::TextureArray)].as_array();
		if (array && !array->empty())
		{
			desc.array.resize(array->size());
			
			for (size_t i = 0; i < array->size(); ++i)
				Load(TOMLNode(*array->get(i)), desc.array[i]);
		}
		else
		{
			desc.array.resize(1);
			Load(TOMLNode(), desc.array[0]);
		}
	}
	
	void TextureDocument::Load(TOMLNode node, TextureSlotDesc& desc)
	{
		if (_svg)
		{
			VectorTextureDesc d;
			Load(node, d);
			desc.variant = d;
		}
		else
		{
			RasterTextureDesc d;
			Load(node, d);
			desc.variant = d;
		}
	}
	
	void TextureDocument::Load(TOMLNode node, RasterTextureDesc& desc)
	{
		Load(node, desc.base);
		PROP_RASTER_TEXTURE(OLY_EDITOR_DESC_IO_LOAD_PROP);
	}
	
	void TextureDocument::Load(TOMLNode node, VectorTextureDesc& desc)
	{
		Load(node, desc.base);
		PROP_VECTOR_TEXTURE(OLY_EDITOR_DESC_IO_LOAD_PROP);
	}
	
	void TextureDocument::Load(TOMLNode node, BaseTextureDesc& desc)
	{
		PROP_TEXTURE_PARAMS(OLY_EDITOR_DESC_IO_LOAD_PROP);

		if (_gif)
			desc.anim = true;
		else
			OLY_EDITOR_DESC_IO_LOAD_PROP(anim, detail::Key::Animated, false);

		Load(node, desc.spritesheet);
	}
	
	void TextureDocument::Load(TOMLNode node, SpritesheetDesc& desc)
	{
		PROP_SPRITESHEET(OLY_EDITOR_DESC_IO_LOAD_PROP);
	}

	void TextureDocument::Dump(toml::table& table, TextureDesc& desc)
	{
		for (TextureSlotDesc& d : desc.array)
			Dump(table, d);
	}

	void TextureDocument::Dump(toml::table& table, TextureSlotDesc& desc)
	{
		std::visit([this, &table](auto& d) { Dump(table, d); }, desc.variant);
	}

	void TextureDocument::Dump(toml::table& table, RasterTextureDesc& desc)
	{
		Dump(table, desc.base);
		PROP_RASTER_TEXTURE(OLY_EDITOR_DESC_IO_DUMP_PROP);
	}

	void TextureDocument::Dump(toml::table& table, VectorTextureDesc& desc)
	{
		Dump(table, desc.base);
		PROP_VECTOR_TEXTURE(OLY_EDITOR_DESC_IO_DUMP_PROP);
	}

	void TextureDocument::Dump(toml::table& table, BaseTextureDesc& desc)
	{
		PROP_TEXTURE_PARAMS(OLY_EDITOR_DESC_IO_DUMP_PROP);

		OLY_EDITOR_DESC_IO_DUMP_PROP(anim, detail::Key::Animated);

		if (desc.anim)
			Dump(table, desc.spritesheet);
	}

	void TextureDocument::Dump(toml::table& table, SpritesheetDesc& desc)
	{
		PROP_SPRITESHEET(OLY_EDITOR_DESC_IO_DUMP_PROP);
	}
}
