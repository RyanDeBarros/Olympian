#pragma once

#include "core/util/Logger.h"

#include "assets/ResourcePath.h"

#include <imp/utf.hpp>

namespace oly
{
    extern Logger::Impl operator<<(Logger::Impl, const imp::utf::string& str);
    extern Logger::Impl operator<<(Logger::Impl, const detail::ResourcePath& file);
}
