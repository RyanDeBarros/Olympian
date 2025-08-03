#pragma once

#include <string>

#include "external/TOML.h"
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

	extern toml::parse_result load_toml(const char* file);
	inline toml::parse_result load_toml(const std::string& file) { return load_toml(file.c_str()); }
}
