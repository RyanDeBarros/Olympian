#pragma once

#include "external/TOML.h"

#include "assets/ResourcePath.h"

namespace oly::io
{
	extern toml::table load_toml(const detail::ResourcePath& file);
}
