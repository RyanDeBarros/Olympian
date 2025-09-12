#pragma once

#include "external/GL.h"

namespace oly::graphics::internal_shaders
{
	extern GLuint sprite_batch;
	extern GLuint polygon_batch;
	extern GLuint ellipse_batch;
		
	extern void load();
	extern void unload();
}
