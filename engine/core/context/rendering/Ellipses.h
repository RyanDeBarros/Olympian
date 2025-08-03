#pragma once

#include "external/TOML.h"

namespace oly::rendering
{
	class EllipseBatch;
}

namespace oly::context
{
	namespace internal
	{
		extern void init_ellipses(const TOMLNode&);
		extern void terminate_ellipses();
	}

	extern rendering::EllipseBatch& ellipse_batch();
	extern void render_ellipses();
}
