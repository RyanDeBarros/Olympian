#pragma once

#include <stb/stb_truetype.h>

#include <unordered_map>
#include <memory>

#include "math/Geometry.h"
#include "util/UTF.h"
#include "../../core/Textures.h"

namespace oly
{
	namespace rendering
	{
		namespace glyphs
		{
			static constexpr const char8_t* COMMON = u8"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,./<>?;:\'\"\\|[]{}!@#$%^&*()-=_+`~";
			static constexpr const char8_t* ALPHA_NUMERIC = u8"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
			static constexpr const char8_t* NUMERIC = u8"0123456789";
			static constexpr const char8_t* ALPHA = u8"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
			static constexpr const char8_t* ALPHA_LOWERCASE = u8"abcdefghijklmnopqrstuvwxyz";
			static constexpr const char8_t* ALPHA_UPPERCASE = u8"ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		}

		struct CodepointPairHash
		{
			size_t operator()(const std::pair<utf::Codepoint, utf::Codepoint>& p) const { return std::hash<int>{}(p.first) ^ (std::hash<int>{}(p.second) << 1); }
		};

		struct Kerning
		{
			typedef std::unordered_map<std::pair<utf::Codepoint, utf::Codepoint>, int, CodepointPairHash> Map; // maps pairs of glyphs to kerning spacing
			Map map;

			Kerning(const char* kerning_file);
		};

		typedef std::shared_ptr<Kerning> KerningRes;

		class FontFace
		{
			std::vector<unsigned char> data;
			stbtt_fontinfo info = {};

		public:
			FontFace(const char* font_file);

			float scale_for_pixel_height(float font_size) const;
			void get_glyph_horizontal_metrics(int glyph_index, int& advance_width, int& left_bearing) const;
			void get_codepoint_horizontal_metrics(utf::Codepoint codepoint, int& advance_width, int& left_bearing) const;
			void get_vertical_metrics(int& ascent, int& descent, int& linegap) const;
			int find_glyph_index(utf::Codepoint codepoint) const;
			int get_kern_advance(int g1, int g2) const;
			void get_bitmap_box(int glyph_index, float scale, int& ch_x0, int& ch_x1, int& ch_y0, int& ch_y1) const;
			void make_bitmap(unsigned char* buf, int w, int h, float scale, int glyph_index) const;
		};

		class FontAtlas;
		struct Glyph
		{
			int index = 0;
			int width = 0, height = 0;
			int ch_y0 = 0;
			int advance_width = 0, left_bearing = 0;
			std::unique_ptr<BindlessTexture> texture;
			size_t buffer_pos = -1;

			Glyph(FontAtlas& font, int index, float scale, size_t buffer_pos);


			void render_on_bitmap_shared(const FontAtlas& font, unsigned char* buffer, int w, int h, int left_padding, int right_padding, int bottom_padding, int top_padding) const;
			void render_on_bitmap_unique(const FontAtlas& font, unsigned char* buffer, int w, int h) const;
		};

		struct FontOptions
		{
			float font_size;
			GLenum min_filter = GL_NEAREST, mag_filter = GL_NEAREST;
			bool auto_generate_mipmaps = false;
		};

		class FontAtlas
		{
			std::shared_ptr<FontFace> font;
			friend struct Glyph;
			std::unordered_map<utf::Codepoint, Glyph> glyphs;
			FontOptions options;
			float scale = 0.0f;
			int ascent = 0, descent = 0, linegap = 0, baseline = 0;
			int space_width = 0;
			ImageDimensions common_dim;
			std::unique_ptr<BindlessTexture> common_texture;
			KerningRes kerning;

		public:
			FontAtlas(const std::shared_ptr<FontFace>& font, FontOptions options, utf::String common_buffer = glyphs::COMMON, const std::shared_ptr<Kerning>& kerning = nullptr);

			bool cache(utf::Codepoint codepoint);
			void cache_all(const FontAtlas& other);
			bool supports(utf::Codepoint codepoint) const;
			int kerning_of(utf::Codepoint c1, utf::Codepoint c2, int g1, int g2, float sc = 1.0f) const;
			int line_height(float line_spacing = 1.0f) const;
			math::Rect2D uvs(const Glyph& glyph) const;
		};
	}
}
