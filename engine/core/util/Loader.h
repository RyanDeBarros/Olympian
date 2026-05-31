#pragma once

#include "external/TOML.h"
#include "core/base/Transforms.h"
#include "core/util/StringParam.h"

#include "assets/ResourcePath.h"

namespace oly::io
{
	extern toml::v3::parse_result load_toml(const detail::ResourcePath& file);
}
