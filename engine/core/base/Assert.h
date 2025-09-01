#pragma once

#include "core/util/Logger.h"

namespace oly::internal
{
	extern bool check_opengl_error();
	extern bool check_glfw_error();
	extern void check_errors();

	constexpr void assertcond(bool condition, const char* description = "", const char* file = nullptr, int line = 0)
	{
		if (!condition)
		{
			if (file)
				OLY_LOG_ERROR(true, "ASSERT") << description << " at " << file << ":" << line << LOG.nl;
			else
				OLY_LOG_ERROR(true, "ASSERT") << description << LOG.nl;
			// LATER configure different kinds of asserts using macros (debug vs release, oly vs game), and whether they should debug break
			__debugbreak();
		}
	}
}

#define OLY_ASSERT(x) { oly::internal::assertcond((x), #x, __FILE__, __LINE__); }
