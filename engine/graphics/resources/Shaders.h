#pragma once

#include "external/GL.h"
#include "graphics/backend/basic/Shader.h"
#include "core/types/SmartReference.h"

namespace oly::graphics::internal_shaders
{
	extern SmartReference<Shader> sprite_batch(GLushort modulations, GLushort anims);
	extern GLuint polygon_batch;
	extern GLuint ellipse_batch;

	extern GLuint particle_renderer;
	extern SmartReference<Shader> particle_compute_spawn(GLushort x_threads);
	extern SmartReference<Shader> particle_compute_update(GLushort x_threads);

	extern void load();
	extern void unload();
}
