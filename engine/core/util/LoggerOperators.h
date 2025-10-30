#pragma once

#include "core/util/Logger.h"

namespace oly
{
	namespace utf
	{
		class String;
	}

	extern Logger::Impl operator<<(Logger::Impl, const utf::String& str);
	extern Logger::Impl operator<<(Logger::Impl, const class ResourcePath& file);
}
