#include "RasterFont.h"

#include "core/context/rendering/Textures.h"

namespace oly::rendering
{
	void RasterFontGlyph::calc_uvs()
	{
		glm::vec2 dimensions = context::get_texture_dimensions(_texture);
		_uvs = {
			.x1 = _box.x1 / dimensions.x,
			.x2 = _box.x2 / dimensions.x,
			.y1 = _box.y1 / dimensions.y,
			.y2 = _box.y2 / dimensions.y
		};
	}
}
