#include "Font.h"

#include "core/base/Errors.h"
#include "core/util/IO.h"
#include "core/util/Logger.h"
#include "core/util/Memory.h"

namespace oly
{
	namespace rendering
	{
		FontFace::FontFace(const char* font_file, Kerning&& kerning)
			: info{}, data(io::read_file_uc(font_file)), kerning(std::move(kerning))
		{
			if (!stbtt_InitFont(&info, data.data(), 0))
			{
				OLY_LOG_ERROR(true, "RENDERING") << LOG.source_info.full_source() << "Cannot initialize font" << LOG.endl;
				throw Error(ErrorCode::LOAD_FONT);
			}
		}

		float FontFace::scale_for_pixel_height(float font_size) const
		{
			return stbtt_ScaleForPixelHeight(&info, font_size);
		}

		void FontFace::get_glyph_horizontal_metrics(int glyph_index, int& advance_width, int& left_bearing) const
		{
			stbtt_GetGlyphHMetrics(&info, glyph_index, &advance_width, &left_bearing);
		}

		void FontFace::get_codepoint_horizontal_metrics(utf::Codepoint codepoint, int& advance_width, int& left_bearing) const
		{
			stbtt_GetCodepointHMetrics(&info, codepoint, &advance_width, &left_bearing);
		}

		void FontFace::get_vertical_metrics(int& ascent, int& descent, int& linegap) const
		{
			stbtt_GetFontVMetrics(&info, &ascent, &descent, &linegap);
		}

		int FontFace::find_glyph_index(utf::Codepoint codepoint) const
		{
			return stbtt_FindGlyphIndex(&info, codepoint);
		}

		void FontFace::get_bitmap_box(int glyph_index, float scale, int& ch_x0, int& ch_x1, int& ch_y0, int& ch_y1) const
		{
			return stbtt_GetGlyphBitmapBox(&info, glyph_index, scale, scale, &ch_x0, &ch_y0, &ch_x1, &ch_y1);
		}

		void FontFace::make_bitmap(unsigned char* buf, int w, int h, float scale, int glyph_index) const
		{
			stbtt_MakeGlyphBitmap(&info, buf, w, h, w, scale, scale, glyph_index);
			flip_pixel_buffer(buf, w, h, 1);
		}

		int FontFace::get_kerning(utf::Codepoint c1, utf::Codepoint c2, int g1, int g2) const
		{
			if (g1 == 0)
				return 0;
			auto k = kerning.map.find({ c1, c2 });
			return k != kerning.map.end() ? k->second : stbtt_GetGlyphKernAdvance(&info, g1, g2);
		}

		int FontFace::get_kerning(utf::Codepoint c1, utf::Codepoint c2) const
		{
			int g1 = find_glyph_index(c1);
			int g2 = find_glyph_index(c2);
			return get_kerning(c1, c2, g1, g2);
		}

		FontGlyph::FontGlyph(FontAtlas& font, int index, float scale, size_t buffer_pos)
			: index(index), buffer_pos(buffer_pos)
		{
			font.font->get_glyph_horizontal_metrics(index, advance_width, left_bearing);
			font.font->get_bitmap_box(index, scale, box.x1, box.x2, box.y1, box.y2);
		}

		void FontGlyph::render_on_bitmap_shared(const FontAtlas& font, unsigned char* buffer, int w, int h, int left_padding, int right_padding, int bottom_padding, int top_padding) const
		{
			int width = box.width();
			int height = box.height();
			unsigned char* temp = new unsigned char[width * height];
			font.font->make_bitmap(temp, width, height, font.scale, index);
			for (int row = 0; row < bottom_padding; ++row)
				memset(buffer + row * w, 0, left_padding + width + right_padding); 
			for (int row = bottom_padding; row < bottom_padding + height; ++row)
			{
				memset(buffer + row * w, 0, left_padding);
				memcpy(buffer + row * w + left_padding, temp + (row - bottom_padding) * width, width);
				memset(buffer + row * w + left_padding + width, 0, right_padding);
			}
			for (int row = height + bottom_padding; row < h; ++row)
				memset(buffer + row * w, 0, left_padding + width + right_padding);
			delete[] temp;
		}

		void FontGlyph::render_on_bitmap_unique(const FontAtlas& font, unsigned char* buffer, int w, int h) const
		{
			font.font->make_bitmap(buffer, box.width(), box.height(), font.scale, index);
		}

		FontAtlas::FontAtlas(const FontFaceRef& font, FontOptions options, const utf::String& common_buffer)
			: font(font), options(options)
		{
			common_dim.cpp = 1;

			scale = font->scale_for_pixel_height(options.font_size);
			font->get_vertical_metrics(ascent, descent, linegap);
			baseline = ascent * scale;

			std::vector<utf::Codepoint> codepoints;
			auto iter = common_buffer.begin();
			while (iter)
			{
				utf::Codepoint codepoint = iter.advance();
				if (codepoint == ' ')
					continue;
				if (glyphs.find(codepoint) != glyphs.end())
					continue;
				int g = font->find_glyph_index(codepoint);
				if (!g)
					continue;
				FontGlyph glyph(*this, g, scale, common_dim.w);
				common_dim.w += glyph.box.width() + 2;
				if (glyph.box.height() > common_dim.h)
					common_dim.h = glyph.box.height();
				glyphs.emplace(codepoint, std::move(glyph));
				codepoints.push_back(codepoint);
			}
			common_dim.h += 2;
			if (common_dim.w > 0)
			{
				unsigned char* common_buf = common_dim.pxnew();
				for (utf::Codepoint codepoint : codepoints)
				{
					auto it = glyphs.find(codepoint);
					it->second.render_on_bitmap_shared(*this, common_buf + it->second.buffer_pos, common_dim.w, common_dim.h, 1, 1, 1, 1);
				}
				graphics::Image common_image(common_buf, common_dim);
				common_texture = graphics::BindlessTextureRef(graphics::load_bindless_texture_2d(common_image, options.auto_generate_mipmaps));
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, options.min_filter);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, options.mag_filter);
				common_texture->set_and_use_handle();
				for (auto& [codepoint, glyph] : glyphs)
					glyph.texture = common_texture;
			}
			int space_advance_width, space_left_bearing;
			font->get_codepoint_horizontal_metrics(utf::Codepoint(' '), space_advance_width, space_left_bearing);
			space_width = space_advance_width * scale;
		}

		bool FontAtlas::cache(utf::Codepoint codepoint)
		{
			if (glyphs.find(codepoint) != glyphs.end())
				return true;
			int index = font->find_glyph_index(codepoint);
			if (!index) return false;

			FontGlyph glyph(*this, index, scale, -1);
			graphics::ImageDimensions dim = { glyph.box.width(), glyph.box.height(), 1};
			unsigned char* bmp = dim.pxnew();
			glyph.render_on_bitmap_unique(*this, bmp, dim.w, dim.h);

			graphics::Image image(bmp, dim);
			graphics::BindlessTexture texture = load_bindless_texture_2d(image, options.auto_generate_mipmaps);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, options.min_filter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, options.mag_filter);
			texture.set_and_use_handle();
			glyph.texture = graphics::BindlessTextureRef(std::move(texture));
			glyphs.emplace(codepoint, std::move(glyph));
			return true;
		}

		void FontAtlas::cache_all(const FontAtlas& other)
		{
			for (const auto& [codepoint, glyph] : other.glyphs)
				cache(codepoint);
		}

		const FontGlyph& FontAtlas::get_glyph(utf::Codepoint codepoint) const
		{
			auto it = glyphs.find(codepoint);
			if (it != glyphs.end())
				return it->second;
			else
				throw Error(ErrorCode::UNCACHED_GLYPH);
		}

		int FontAtlas::get_glyph_index(utf::Codepoint codepoint) const
		{
			auto it = glyphs.find(codepoint);
			if (it != glyphs.end())
				return it->second.index;
			else
				return font->find_glyph_index(codepoint);
		}

		bool FontAtlas::supports(utf::Codepoint codepoint) const
		{
			if (glyphs.find(codepoint) != glyphs.end())
				return true;
			return font->find_glyph_index(codepoint) != 0;
		}

		float FontAtlas::kerning_of(utf::Codepoint c1, utf::Codepoint c2, int g1, int g2) const
		{
			return font->get_kerning(c1, c2, g1, g2) * scale;
		}

		float FontAtlas::kerning_of(utf::Codepoint c1, utf::Codepoint c2) const
		{
			return font->get_kerning(c1, c2) * scale;
		}

		float FontAtlas::line_height() const
		{
			return (ascent - descent + linegap) * scale;
		}

		float FontAtlas::get_ascent() const
		{
			return ascent * scale;
		}

		float FontAtlas::get_descent() const
		{
			return descent * scale;
		}

		float FontAtlas::get_linegap() const
		{
			return linegap * scale;
		}

		math::Rect2D FontAtlas::uvs(const FontGlyph& glyph) const
		{
			math::Rect2D b{};
			if (glyph.buffer_pos != size_t(-1))
			{
				b.x1 = float(glyph.buffer_pos + 1) / common_dim.w;
				b.x2 = float(glyph.buffer_pos + 1 + glyph.box.width()) / common_dim.w;
				b.y1 = 0.0f;
				b.y2 = float(glyph.box.height()) / common_dim.h;
			}
			else
			{
				b.x1 = 0;
				b.x2 = 1;
				b.y1 = 0;
				b.y2 = 1;
			}
			return b;
		}
	}
}
