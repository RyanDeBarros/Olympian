#pragma once

#include "core/base/SimpleMath.h"

namespace oly::context
{
	class Context
	{
	public:
		Context(const char* project_file, const char* resource_root);
		Context(const Context&);
		Context(Context&&) noexcept;
		~Context();
		Context& operator=(const Context&);
		Context& operator=(Context&&) noexcept;
	};

	namespace internal
	{
		extern bool render_frame();
	}

	extern void run();
}
