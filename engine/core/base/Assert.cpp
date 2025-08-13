#include "Assert.h"

namespace oly::internal
{
	bool check_opengl_error()
	{
		if (auto err = glGetError())
		{
			const char* description = "";
			switch (err)
			{
			case GL_INVALID_ENUM:
				description = "invalid enum";
				break;
			case GL_INVALID_VALUE:
				description = "invalid value";
				break;
			case GL_INVALID_OPERATION:
				description = "invalid operation";
				break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				description = "invalid framebuffer operation";
				break;
			case GL_OUT_OF_MEMORY:
				description = "out of memory";
				break;
			case GL_STACK_UNDERFLOW:
				description = "stack underflow";
				break;
			case GL_STACK_OVERFLOW:
				description = "stack overflow";
				break;
			}
			LOG.error(LOG.opengl(err)) << description << LOG.nl;
#ifdef _DEBUG
			__debugbreak();
#endif
			return true;
		}
		return false;
	}

	bool check_glfw_error()
	{
		const char* description = "";
		if (auto err = glfwGetError(&description))
		{
			LOG.error(LOG.glfw(err)) << description << LOG.nl;
#ifdef _DEBUG
			__debugbreak();
#endif
			return true;
		}
		return false;
	}

	void check_errors()
	{
		while (check_opengl_error());
		while (check_glfw_error());
	}
}
