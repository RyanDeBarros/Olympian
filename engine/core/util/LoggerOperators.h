#pragma once

#include "core/util/Logger.h"

namespace oly
{
	namespace utf
	{
		class String;
	}

	namespace detail
	{
		class ResourcePath;
	}

	extern Logger::Impl operator<<(Logger::Impl, const utf::String& str);
	extern Logger::Impl operator<<(Logger::Impl, const detail::ResourcePath& file);
}
