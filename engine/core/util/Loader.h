#pragma once

#include "external/TOML.h"
#include "core/base/Transforms.h"
#include "core/util/StringParam.h"

#include "assets/ResourcePath.h"

namespace oly::io
{
	extern toml::table load_toml(const detail::ResourcePath& file);
}
