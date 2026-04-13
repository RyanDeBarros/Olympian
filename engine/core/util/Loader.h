#pragma once

#include "external/TOML.h"
#include "core/base/Transforms.h"
#include "core/util/ResourcePath.h"
#include "core/util/StringParam.h"

namespace oly::io
{
	extern toml::v3::parse_result load_toml(const ResourcePath& file);

	// TODO v7 move to Transform
	extern Polymorphic<TransformModifier2D> load_transform_modifier_2d(TOMLNode node);

	// TODO v7 move to different file
	extern bool parse_color(const StringParam& text, glm::vec4& color);
}
