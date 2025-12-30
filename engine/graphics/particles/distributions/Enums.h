#pragma once

#include "graphics/particles/Attribute.h"
#include "core/util/Enum.h"

namespace oly::particles
{
	// TODO v6 rename 'distributions' folder

#define _DirectionEnumEntryMap(X) X(NONE, 0) X(RIGHT, 1) X(LEFT, 2)
	OLY_ENUM(DirectionEnum, _DirectionEnumEntryMap);
#undef _DirectionEnumEntryMap
}
