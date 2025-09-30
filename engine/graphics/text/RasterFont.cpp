#include "RasterFont.h"

#include "core/context/rendering/Textures.h"

namespace oly::rendering
{
	RasterFontGlyph::RasterFontGlyph(const graphics::BindlessTextureRef& texture, math::IRect2D location, glm::vec2 origin_offset, math::Padding padding)
		: _texture(texture)
	{
		const glm::vec2 dimensions = context::get_texture_dimensions(_texture);

		_uvs = {
			.x1 = location.x1 / dimensions.x,
			.x2 = (location.x2 + 1) / dimensions.x,
			.y1 = location.y1 / dimensions.y,
			.y2 = (location.y2 + 1) / dimensions.y
		};

		_box = {
			.x1 = -origin_offset.x,
			.x2 = location.width() + 1 - origin_offset.x,
			.y1 = -origin_offset.y,
			.y2 = location.height() + 1 - origin_offset.y
		};

		_left_bearing = padding.left;
		_advance_width = _box.width() + padding.left + padding.right;
		// TODO v5 use vertical padding?
	}
}
