#pragma once

#include "external/GL.h"

namespace oly::graphics::internal_shaders
{
	extern GLuint sprite_batch(GLushort modulations, GLushort anims);
	extern GLuint polygon_batch;
	extern GLuint ellipse_batch;

	extern GLuint particle_renderer;
	extern GLuint particle_compute_spawn;
	extern GLuint particle_compute_update;

	extern void load();
	extern void unload();
}
