#pragma once

#include "Logger.h"

namespace oly
{
	extern bool check_opengl_error();
	extern bool check_glfw_error();
	extern void check_errors();

	constexpr void assertcond(bool condition, const char* description = "", const char* file = nullptr, int line = 0)
	{
		if (!condition)
		{
			if (file)
				LOG << LOG.begin_temp(LOG.error) << LOG.start_prefix("ASSERT") << description << " at " << file << ":" << line << LOG.end_temp << LOG.nl;
			else
				LOG << LOG.begin_temp(LOG.error) << LOG.start_prefix("ASSERT") << description << LOG.end_temp << LOG.nl;
			// LATER configure different kinds of asserts using macros (debug vs release, oly vs game), and whether they should debug break
			__debugbreak();
		}
	}
}

#define OLY_ASSERT(x) { oly::assertcond((x), #x, __FILE__, __LINE__); }
