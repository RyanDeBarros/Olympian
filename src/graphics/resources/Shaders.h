#pragma once

#include <string>

#include "external/GL.h"

namespace oly
{
	namespace shaders
	{
		extern GLuint sprite_batch;
		extern GLuint polygon_batch;
		extern GLuint ellipse_batch;
		extern GLuint text_batch;
		extern GLuint polygonal_particle;
		extern GLuint elliptic_particle;
		
		extern GLuint location(GLuint shader, const std::string& uniform);

		extern void load();
		extern void unload();
	}
}
