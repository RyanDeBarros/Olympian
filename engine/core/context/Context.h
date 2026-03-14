#pragma once

#include "core/base/SimpleMath.h"

namespace oly::context
{
	class Context
	{
	public:
		Context(const char* project_file, const char* resource_root);
		Context(const Context&) = delete;
		Context(Context&&) noexcept = delete;
		~Context();
	};

	namespace internal
	{
		extern bool render_frame();
	}

	extern void run();
}
