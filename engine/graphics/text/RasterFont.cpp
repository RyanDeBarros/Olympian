#include "RasterFont.h"

#include "core/context/rendering/Textures.h"

namespace oly::rendering
{
	RasterFontGlyph::RasterFontGlyph(
        const graphics::BindlessTextureRef& texture,
        math::IRect2D location,
		math::TopSidePadding padding,
        detail::PositioningMode origin_offset_mode,
        glm::vec2 origin_offset)
		: texture_(texture)
	{
		const glm::vec2 dimensions = context::get_texture_size(texture_);

		uvs_ = {
			.x1 = location.x1 / dimensions.x,
			.x2 = (location.x2 + 1) / dimensions.x,
			.y1 = location.y1 / dimensions.y,
			.y2 = (location.y2 + 1) / dimensions.y
		};

		if (origin_offset_mode == detail::PositioningMode::Relative)
		{
			origin_offset = {
				(location.width() + 1.0f) * origin_offset.x,
				(location.height() + 1.0f) * (1.0f - origin_offset.y),
			};
		}

		box_ = {
			.x1 = -origin_offset.x,
			.x2 = location.width() + 1 - origin_offset.x,
			.y1 = -origin_offset.y - padding.top,
			.y2 = location.height() + 1 - origin_offset.y - padding.top
		};

		left_bearing_ = padding.left;
		advance_width_ = box_.width() + padding.left + padding.right;
	}

    graphics::BindlessTextureRef RasterFontGlyph::texture() const
    {
        return texture_;
    }

    math::Rect2D RasterFontGlyph::box() const
    {
        return box_;
    }

    math::UVRect RasterFontGlyph::uvs() const
    {
        return uvs_;
    }

    float RasterFontGlyph::advance_width() const
    {
        return advance_width_;
    }

    float RasterFontGlyph::left_bearing() const
    {
        return left_bearing_;
    }

    RasterFont::RasterFont(
        std::unordered_map<imp::utf::codepoint, RasterFontGlyph>&& glyphs,
        float space_advance_width,
        float line_height,
        glm::vec2 font_scale,
        Kerning&& kerning )
        : glyphs_(std::move(glyphs))
        , font_scale_(font_scale)
        , kerning_(std::move(kerning))
        , line_height_(line_height)
        , space_advance_width_(space_advance_width)
    {
    }

    const RasterFontGlyph& RasterFont::get_glyph(imp::utf::codepoint codepoint) const
    {
        auto it = glyphs_.find(codepoint);
        if (it != glyphs_.end())
            return it->second;
        else
            throw Error(ErrorCode::UncachedGlyph);
    }

    bool RasterFont::supports(imp::utf::codepoint codepoint) const
    {
        return glyphs_.count(codepoint);
    }

    float RasterFont::kerning_of(imp::utf::codepoint c1, imp::utf::codepoint c2) const
    {
        auto it = kerning_.map.find({ c1, c2 });
        return it != kerning_.map.end() ? it->second * font_scale_.x : 0.0f;
    }

    float RasterFont::line_height() const
    {
        return font_scale_.y * line_height_;
    }

    glm::vec2 RasterFont::get_scale() const
    {
        return font_scale_;
    }

    void RasterFont::set_scale(glm::vec2 scale)
    {
        font_scale_ = scale;
    }

    float RasterFont::get_scaled_space_advance_width() const
    {
        return space_advance_width_ * font_scale_.x;
    }
}
