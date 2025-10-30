#pragma once

#include "external/STB.h"

#include "core/types/SmartReference.h"
#include "core/math/Shapes.h"
#include "core/util/UTF.h"

#include "graphics/backend/basic/Textures.h"
#include "graphics/text/Kerning.h"

namespace oly::rendering
{
	namespace glyphs
	{
		static constexpr const char8_t* COMMON = u8"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,./<>?;:\'\"\\|[]{}!@#$%^&*()-=_+`~";
		static constexpr const char8_t* ALPHA_NUMERIC = u8"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
		static constexpr const char8_t* NUMERIC = u8"0123456789";
		static constexpr const char8_t* ALPHABET = u8"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
		static constexpr const char8_t* ALPHABET_LOWERCASE = u8"abcdefghijklmnopqrstuvwxyz";
		static constexpr const char8_t* ALPHABET_UPPERCASE = u8"ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	}

	class FontFace
	{
		std::vector<unsigned char> data;
		stbtt_fontinfo info = {};
		Kerning kerning;

	public:
		FontFace(const ResourcePath& font_file, Kerning&& kerning);

		float scale_for_pixel_height(float font_size) const;
		void get_glyph_horizontal_metrics(int glyph_index, int& advance_width, int& left_bearing) const;
		void get_codepoint_horizontal_metrics(utf::Codepoint codepoint, int& advance_width, int& left_bearing) const;
		void get_vertical_metrics(int& ascent, int& descent, int& linegap) const;
		int find_glyph_index(utf::Codepoint codepoint) const;
		void get_bitmap_box(int glyph_index, float scale, int& ch_x0, int& ch_x1, int& ch_y0, int& ch_y1) const;
		void make_bitmap(unsigned char* buf, int w, int h, float scale, int glyph_index) const;
		int get_kerning(utf::Codepoint c1, utf::Codepoint c2) const;
	};

	typedef SmartReference<FontFace> FontFaceRef;

	// TODO v7 manual generation of mipmaps

	class FontAtlas;
	class FontGlyph
	{
		int index = 0;
		math::IRect2D _box;
		int _advance_width = 0, _left_bearing = 0;
		graphics::BindlessTextureRef _texture;
		size_t buffer_pos = -1;

	public:
		FontGlyph(const FontAtlas& font, int index, float scale, size_t buffer_pos);

		math::IRect2D box() const { return _box; }
		int advance_width() const { return _advance_width; }
		int left_bearing() const { return _left_bearing; }
		graphics::BindlessTextureRef texture() const { return _texture; }

	private:
		friend class FontAtlas;
		void render_on_bitmap_shared(const FontAtlas& font, unsigned char* buffer, int w, int h, int left_padding, int right_padding, int bottom_padding, int top_padding) const;
		void render_on_bitmap_unique(const FontAtlas& font, unsigned char* buffer, int w, int h) const;
	};

	struct FontOptions
	{
		float font_size = 36.0f;
		GLenum min_filter = GL_NEAREST, mag_filter = GL_NEAREST;
		bool auto_generate_mipmaps = false;
	};

	class FontAtlas
	{
		FontFaceRef font;
		friend struct FontGlyph;
		mutable std::unordered_map<utf::Codepoint, FontGlyph> glyphs;
		FontOptions options;
		float scale = 1.0f;
		float _line_height = 0.0f;
		int ascent = 0;
		float space_advance_width = 0.0f;
		graphics::ImageDimensions common_dim;
		graphics::BindlessTextureRef common_texture;

	public:
		FontAtlas(const FontFaceRef& font, FontOptions options, const utf::String& common_buffer = glyphs::COMMON);

		const FontFaceRef& font_face() const { return font; }

		bool cache(utf::Codepoint codepoint) const;
		void cache_all(const FontAtlas& other) const;
		const FontGlyph& get_glyph(utf::Codepoint codepoint) const;
		int get_glyph_index(utf::Codepoint codepoint) const;
		bool supports(utf::Codepoint codepoint) const;
		float kerning_of(utf::Codepoint c1, utf::Codepoint c2) const;
		float line_height() const;
		float get_ascent() const;
		math::UVRect uvs(const FontGlyph& glyph) const;
		float get_scale() const { return scale; }
		float get_scaled_space_advance_width() const { return space_advance_width; }
	};

	typedef SmartReference<FontAtlas> FontAtlasRef;
}
