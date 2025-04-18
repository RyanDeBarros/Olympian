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
		extern GLuint texture_quad_batch;
		extern GLuint polygon_batch;
		extern GLuint ellipse_batch;
		extern GLuint polygonal_particle;
		extern GLuint elliptic_particle;
		
		extern GLuint location(GLuint shader, const std::string& uniform);
		extern void load();
		extern void unload();
	}

	extern void load_resources();
	extern void unload_resources();
}
