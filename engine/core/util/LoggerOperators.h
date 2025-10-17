#pragma once

#include "core/util/Logger.h"

#include "core/util/UTF.h"

namespace oly
{
	extern Logger::Impl operator<<(Logger::Impl, const utf::String& str);
}
