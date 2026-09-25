#pragma once

#include "core/util/Logger.h"

#include <imp/utf.hpp>

namespace oly
{
	namespace detail
	{
		class ResourcePath;
	}

	extern Logger::Impl operator<<(Logger::Impl, const imp::utf::string& str);
	extern Logger::Impl operator<<(Logger::Impl, const detail::ResourcePath& file);
}
