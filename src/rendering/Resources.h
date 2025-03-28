#pragma once

#include "core/Core.h"

namespace oly
{
	namespace samplers
	{
		extern GLuint linear;
		extern GLuint nearest;

		extern void load();
		extern void unload();
	}

	namespace shaders
	{
		extern GLuint sprite_batch;
		extern GLuint polygon_batch;
		extern GLuint ellipse_batch;
		
		extern void load();
		extern void unload();
	}

	extern void load_resources();
	extern void unload_resources();
}
