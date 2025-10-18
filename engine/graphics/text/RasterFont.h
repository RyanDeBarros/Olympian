#pragma once

#include "graphics/backend/basic/Textures.h"
#include "graphics/text/Kerning.h"

namespace oly::rendering
{
	class RasterFontGlyph
	{
		graphics::BindlessTextureRef _texture;
		math::Rect2D _box;
		math::UVRect _uvs;
		float _advance_width = 0.0f, _left_bearing = 0.0f;

	public:
		RasterFontGlyph(const graphics::BindlessTextureRef& texture, math::IRect2D location,
			math::TopSidePadding padding = {}, math::PositioningMode origin_offset_mode = math::PositioningMode::RELATIVE, glm::vec2 origin_offset = {});

		graphics::BindlessTextureRef texture() const
		{
			return _texture;
		}

		math::Rect2D box() const
		{
			return _box;
		}

		math::UVRect uvs() const
		{
			return _uvs;
		}

		float advance_width() const
		{
			return _advance_width;
		}

		float left_bearing() const
		{
			return _left_bearing;
		}
	};

	class RasterFont
	{
		Kerning kerning;
		std::unordered_map<utf::Codepoint, RasterFontGlyph> glyphs;
		glm::vec2 font_scale = glm::vec2(1.0f);
		float _line_height = 0.0f, _space_advance_width = 0.0f;

	public:
		RasterFont(std::unordered_map<utf::Codepoint, RasterFontGlyph>&& glyphs, float space_advance_width, float line_height, glm::vec2 font_scale = glm::vec2(1.0f), Kerning&& kerning = {})
			: glyphs(std::move(glyphs)), font_scale(font_scale), kerning(std::move(kerning)), _line_height(line_height), _space_advance_width(space_advance_width)
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
		
		glm::vec2 get_scale() const
		{
			return font_scale;
		}
		
		void set_scale(glm::vec2 scale)
		{
			font_scale = scale;
		}
		
		float get_scaled_space_advance_width() const
		{
			return _space_advance_width * font_scale.x;
		}
	};

	typedef SmartReference<RasterFont> RasterFontRef;
}
