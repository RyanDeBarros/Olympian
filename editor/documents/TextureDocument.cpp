#include "TextureDocument.h"

#include <imgui.h>

#include "desc/DescIO.h"

namespace oly::editor
{
	void TextureDocument::Init()
	{
		// TODO v7 load _desc
	}

	void TextureDocument::Draw()
	{
		Draw(_desc);
	}

	detail::ResourcePath TextureDocument::GetSourcePath() const
	{
		return _oly_path.get_source_path();
	}

	void TextureDocument::Draw(TextureArrayDesc& desc)
	{
		for (TextureDesc& d : desc.array)
			Draw(d);
	}
	
	void TextureDocument::Draw(TextureDesc& desc)
	{
		std::visit([this](auto& d) { Draw(d); }, desc.variant);
	}
	
	void TextureDocument::Draw(RasterTextureDesc& desc)
	{
		Draw(desc.base);

		if (DescIO::Draw("Generate Mipmaps", desc.generate_mipmaps))
			MarkDirty();

		if (DescIO::Draw("Storage", desc.storage))
			MarkDirty();
	}
	
	void TextureDocument::Draw(VectorTextureDesc& desc)
	{
		Draw(desc.base);

		if (DescIO::Draw("Generate Mipmaps", desc.generate_mipmaps))
			MarkDirty();

		if (DescIO::Draw("Image Storage", desc.image_storage))
			MarkDirty();

		if (DescIO::Draw("Abstract Storage", desc.abstract_storage))
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

		if (DescIO::Draw("Animated", desc.anim))
			MarkDirty();

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
}
