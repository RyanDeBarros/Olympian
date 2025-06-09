#include "CollisionView.h"

#include "core/base/Context.h"
#include "graphics/extensions/Arrow.h"

namespace oly::debug
{
	void render_collision()
	{
		context::render_ellipses();
		context::render_polygons();
	}

	void draw_impulse(const col2d::ContactResult::Feature& feature, glm::vec4 color)
	{
		oly::rendering::ArrowExtension impulse; // TODO StaticArrowExtension that doesn't have transformer
		impulse.set_color(color);
		impulse.adjust_standard_head_for_width(6.0f);
		impulse.set_start() = feature.position;
		impulse.set_end() = feature.position + feature.impulse;
		impulse.draw();
	}
}
