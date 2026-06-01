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
		Draw(_desc);
	}

	void TextureDocument::Load()
	{
		if (_oly_path.exists())
		{
			toml::table table;
			std::string err = _oly_path.load_toml(table);
			if (err.empty())
				Load(TOMLNode(table), _desc);
			else
			{
				// TODO v7 log error - corrupted asset
			}
		}
		else
			Load(TOMLNode(), _desc);
	}

	void TextureDocument::Dump()
	{
		// TODO v7 Dump(_desc)
		MarkClean();
	}

	detail::ResourcePath TextureDocument::GetSourcePath() const
	{
		return _oly_path.get_source_path();
	}

	void TextureDocument::Draw(TextureArrayDesc& desc)
	{
		// TODO v7 combo box to select slot, buttons to create new, delete.
		for (TextureDesc& d : desc.array)
			Draw(d);
	}
	
	void TextureDocument::Draw(TextureDesc& desc)
	{
		if (DescIO::BeginForm(&desc))
		{
			std::visit([this](auto& d) { Draw(d); }, desc.variant);
			DescIO::EndForm();
		}
	}
	
	void TextureDocument::Draw(RasterTextureDesc& desc)
	{
		Draw(desc.base);

		if (DescIO::Draw("Storage", desc.storage))
			MarkDirty();

		if (DescIO::Draw("Generate Mipmaps", desc.generate_mipmaps))
			MarkDirty();
	}
	
	void TextureDocument::Draw(VectorTextureDesc& desc)
	{
		Draw(desc.base);

		if (DescIO::Draw("Vector Scale", desc.scale, 0.f, std::nullopt))
			MarkDirty();

		if (DescIO::Draw("Image Storage", desc.image_storage))
			MarkDirty();

		if (DescIO::Draw("Abstract Storage", desc.abstract_storage))
			MarkDirty();

		if (DescIO::Draw("Generate Mipmaps", desc.generate_mipmaps))
			MarkDirty();
	}
	
	void TextureDocument::Draw(BaseTextureDesc& desc)
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

		if (DescIO::Draw("Min Filter", desc.min_filter, min_filter_values, min_filter_names, IM_ARRAYSIZE(min_filter_names)))
			MarkDirty();

		static const GLenum mag_filter_values[] = {
			GL_NEAREST,
			GL_LINEAR,
		};

		static const char* mag_filter_names[] = {
			"Nearest",
			"Linear",
		};

		if (DescIO::Draw("Mag Filter", desc.mag_filter, mag_filter_values, mag_filter_names, IM_ARRAYSIZE(mag_filter_names)))
			MarkDirty();

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

		if (DescIO::Draw("Wrap (S)", desc.wrap_s, wrap_values, wrap_names, IM_ARRAYSIZE(wrap_names)))
			MarkDirty();

		if (DescIO::Draw("Wrap (T)", desc.wrap_t, wrap_values, wrap_names, IM_ARRAYSIZE(wrap_names)))
			MarkDirty();

		ImGui::BeginDisabled(_gif);
		if (DescIO::Draw("Animated", desc.anim))
			MarkDirty();
		ImGui::EndDisabled();

		if (desc.anim)
			Draw(desc.spritesheet);
	}
	
	void TextureDocument::Draw(SpritesheetDesc& desc)
	{
		if (DescIO::Draw("Rows", desc.rows, 1, std::nullopt))
			MarkDirty();

		if (DescIO::Draw("Columns", desc.cols, 1, std::nullopt))
			MarkDirty();

		if (DescIO::Draw("Cell Width Override", desc.cell_width_override, 0, std::nullopt))
			MarkDirty();

		if (DescIO::Draw("Cell Height Override", desc.cell_height_override, 0, std::nullopt))
			MarkDirty();

		if (DescIO::Draw("Delay (CS)", desc.delay_cs, 0, std::nullopt))
			MarkDirty();

		if (DescIO::Draw("Row Major", desc.row_major))
			MarkDirty();

		if (DescIO::Draw("Row Up", desc.row_up))
			MarkDirty();
	}

	void TextureDocument::Load(TOMLNode node, TextureArrayDesc& desc)
	{
		desc.array.clear();
		
		TOMLArray array = node[detail::decode_key(detail::Key::TextureArray)].as_array();
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
	
	void TextureDocument::Load(TOMLNode node, TextureDesc& desc)
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

		DescIO::Load(node, desc.generate_mipmaps, detail::Key::GenerateMipmaps, false);
		DescIO::Load(node, desc.storage, detail::Key::Storage, detail::StorageMode::Discard);
	}
	
	void TextureDocument::Load(TOMLNode node, VectorTextureDesc& desc)
	{
		Load(node, desc.base);

		DescIO::Load(node, desc.scale, detail::Key::VectorScale, 1.f);
		DescIO::Load(node, desc.generate_mipmaps, detail::Key::GenerateMipmaps, detail::SVGMipmapGenerationMode::Off);
		DescIO::Load(node, desc.image_storage, detail::Key::ImageStorage, detail::StorageMode::Discard);
		DescIO::Load(node, desc.abstract_storage, detail::Key::AbstractStorage, detail::StorageMode::Discard);
	}
	
	void TextureDocument::Load(TOMLNode node, BaseTextureDesc& desc)
	{
		DescIO::Load(node, desc.min_filter, detail::Key::MinFilter, GL_NEAREST);
		DescIO::Load(node, desc.mag_filter, detail::Key::MagFilter, GL_NEAREST);
		DescIO::Load(node, desc.wrap_s, detail::Key::WrapS, GL_CLAMP_TO_EDGE);
		DescIO::Load(node, desc.wrap_t, detail::Key::WrapT, GL_CLAMP_TO_EDGE);

		if (_gif)
			desc.anim = true;
		else
			DescIO::Load(node, desc.anim, detail::Key::Animated, false);

		Load(node, desc.spritesheet);
	}
	
	void TextureDocument::Load(TOMLNode node, SpritesheetDesc& desc)
	{
		DescIO::Load(node, desc.rows, detail::Key::Rows, 1);
		DescIO::Load(node, desc.cols, detail::Key::Columns, 1);
		DescIO::Load(node, desc.cell_width_override, detail::Key::CellWidthOverride, 0);
		DescIO::Load(node, desc.cell_height_override, detail::Key::CellHeightOverride, 0);
		DescIO::Load(node, desc.delay_cs, detail::Key::DelayCS, 0);
		DescIO::Load(node, desc.row_major, detail::Key::RowMajor, true);
		DescIO::Load(node, desc.row_up, detail::Key::RowUp, true);
	}
}
