#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb/stb_image.h>

#include <concepts>

namespace oly
{
	extern void init();
	extern void load_context();
	extern int terminate();
	extern void pre_frame();
}
