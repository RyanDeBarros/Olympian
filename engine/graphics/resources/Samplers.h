#pragma once

#include "external/GL.h"

namespace oly::graphics::samplers
{
	extern GLuint linear;
	extern GLuint nearest;
	extern GLuint linear_mipmap_linear;
	extern GLuint linear_mipmap_nearest;
	extern GLuint nearest_mipmap_linear;
	extern GLuint nearest_mipmap_nearest;

	namespace internal
	{
		extern void load();
		extern void unload();
	}
}
