#pragma once

#include "graphics/backend/basic/Textures.h"
#include "graphics/text/Kerning.h"

namespace oly::rendering
{
	// TODO v5 registry for RasterFont

	class RasterFontGlyph
	{
		graphics::BindlessTextureRef _texture;
		math::Rect2D _box;
		math::UVRect _uvs;

	public:
		int advance_width = 0, left_bearing = 0;

		RasterFontGlyph(const graphics::BindlessTextureRef& texture, const math::Rect2D& box, int advance_width = 0, int left_bearing = 0)
			: _texture(texture), advance_width(advance_width), left_bearing(left_bearing)
		{
			set_box(box);
		}

		graphics::BindlessTextureRef texture() const
		{
			return _texture;
		}

		void set_texture(const graphics::BindlessTextureRef& texture)
		{
			_texture = texture;
			calc_uvs();
		}

		math::Rect2D box() const
		{
			return _box;
		}

		void set_box(const math::Rect2D& box)
		{
			_box = box;
			calc_uvs();
		}

		math::UVRect uvs() const
		{
			return _uvs;
		}

	private:
		void calc_uvs();
	};

	class RasterFont
	{
		Kerning kerning;
		std::unordered_map<utf::Codepoint, RasterFontGlyph> glyphs;
		glm::vec2 font_scale = glm::vec2(1.0f);
		float _line_height = 0.0f, _ascent = 0.0f, _space_advance_width = 0.0f;

	public:
		RasterFont(std::unordered_map<utf::Codepoint, RasterFontGlyph>&& glyphs, float space_advance_width, float line_height, float ascent, glm::vec2 font_scale = glm::vec2(1.0f), Kerning&& kerning = {})
			: glyphs(std::move(glyphs)), font_scale(font_scale), kerning(std::move(kerning)), _line_height(line_height), _ascent(ascent), _space_advance_width(space_advance_width)
		{
		}

		const RasterFontGlyph& get_glyph(utf::Codepoint codepoint) const
		{
			auto it = glyphs.find(codepoint);
			if (it != glyphs.end())
				return it->second;
			else
				throw Error(ErrorCode::UNCACHED_GLYPH);
		}
		
		bool supports(utf::Codepoint codepoint) const
		{
			return glyphs.count(codepoint);
		}
		
		float kerning_of(utf::Codepoint c1, utf::Codepoint c2) const
		{
			auto it = kerning.map.find({ c1, c2 });
			return it != kerning.map.end() ? it->second * font_scale.x : 0.0f;
		}
		
		float line_height() const
		{
			return font_scale.y * _line_height;
		}
		
		float get_ascent() const
		{
			return font_scale.y * _ascent;
		}

		glm::vec2 get_scale() const
		{
			return font_scale;
		}
		
		void set_scale(glm::vec2 scale)
		{
			font_scale = scale;
		}
		
		float get_space_advance_width() const
		{
			return _space_advance_width * font_scale.x;
		}
	};

	typedef SmartReference<RasterFont> RasterFontRef;
}
