#pragma once

#include "graphics/backend/basic/Textures.h"

#include "graphics/text/Kerning.h"

#include "definitions/enums/PositioningMode.h"

namespace oly::rendering
{
	class RasterFontGlyph
	{
		graphics::BindlessTextureRef texture_;
		math::Rect2D box_;
		math::UVRect uvs_;
        float advance_width_ = 0.f;
        float left_bearing_ = 0.f;

	public:
		RasterFontGlyph(
            const graphics::BindlessTextureRef& texture,
            math::IRect2D location,
			math::TopSidePadding padding = {},
            detail::PositioningMode origin_offset_mode = detail::PositioningMode::Relative,
            glm::vec2 origin_offset = {});

        graphics::BindlessTextureRef texture() const;
        math::Rect2D box() const;
        math::UVRect uvs() const;
        float advance_width() const;
        float left_bearing() const;
	};

	class RasterFont
	{
		Kerning kerning_;
		std::unordered_map<imp::utf::codepoint, RasterFontGlyph> glyphs_;
		glm::vec2 font_scale_ = glm::vec2(1.f);
        float line_height_ = 0.f;
        float space_advance_width_ = 0.f;

	public:
        RasterFont(
            std::unordered_map<imp::utf::codepoint, RasterFontGlyph>&& glyphs,
            float space_advance_width,
            float line_height,
            glm::vec2 font_scale = glm::vec2(1.0f),
            Kerning&& kerning = {});

        const RasterFontGlyph& get_glyph(imp::utf::codepoint codepoint) const;
        bool supports(imp::utf::codepoint codepoint) const;
        float kerning_of(imp::utf::codepoint c1, imp::utf::codepoint c2) const;
        float line_height() const;
        glm::vec2 get_scale() const;
        void set_scale(glm::vec2 scale);
        float get_scaled_space_advance_width() const;
	};

	using RasterFontRef = SmartReference<RasterFont>;
}
