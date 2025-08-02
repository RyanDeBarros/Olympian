#pragma once

#include <string>

#include "core/base/SimpleMath.h"

namespace oly::context
{
	class Context
	{
	public:
		Context(const char* resource_root);
		Context(const Context&);
		Context(Context&&) noexcept;
		~Context();
		Context& operator=(const Context&);
		Context& operator=(Context&&) noexcept;
	};

	extern std::string resource_file(const std::string& file);

	extern bool frame();
	extern BigSize this_frame();
}
