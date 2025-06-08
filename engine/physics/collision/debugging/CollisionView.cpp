#include "CollisionView.h"

#include "core/base/Context.h"

namespace oly::debug
{
	void render_collision()
	{
		context::render_ellipses();
		context::render_polygons();
	}
}
