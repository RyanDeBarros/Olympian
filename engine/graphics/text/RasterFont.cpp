#include "RasterFont.h"

#include "core/context/rendering/Textures.h"

namespace oly::rendering
{
	RasterFontGlyph::RasterFontGlyph(const graphics::BindlessTextureRef& texture, math::IRect2D location,
		math::TopSidePadding padding, math::PositioningMode origin_offset_mode, glm::vec2 origin_offset)
		: _texture(texture)
	{
		const glm::vec2 dimensions = context::get_texture_dimensions(_texture);

		_uvs = {
			.x1 = location.x1 / dimensions.x,
			.x2 = (location.x2 + 1) / dimensions.x,
			.y1 = location.y1 / dimensions.y,
			.y2 = (location.y2 + 1) / dimensions.y
		};

		if (origin_offset_mode == math::PositioningMode::RELATIVE)
		{
			origin_offset = {
				(location.width() + 1.0f) * origin_offset.x,
				(location.height() + 1.0f) * (1.0f - origin_offset.y),
			};
		}

		_box = {
			.x1 = -origin_offset.x,
			.x2 = location.width() + 1 - origin_offset.x,
			.y1 = -origin_offset.y - padding.top,
			.y2 = location.height() + 1 - origin_offset.y - padding.top
		};

		_left_bearing = padding.left;
		_advance_width = _box.width() + padding.left + padding.right;
	}
}
