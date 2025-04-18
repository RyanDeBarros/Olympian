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
			auto prev_level = LOG.level;
			LOG.level = LOG.error;
			if (file)
				LOG << LOG.start_prefix("ASSERT") << description << " at " << file << ":" << line << LOG.nl;
			else
				LOG << LOG.start_prefix("ASSERT") << description << LOG.nl;
			LOG.level = prev_level;
			// LATER configure different kinds of asserts using macros (debug vs release, oly vs game), and whether they should debug break
			__debugbreak();
		}
	}
}

#define OLY_ASSERT(x) oly::assertcond((x), #x, __FILE__, __LINE__);
