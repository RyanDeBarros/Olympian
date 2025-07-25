#pragma once

#include "physics/collision/elements/Element.h"

namespace oly::physics
{
	extern float moment_of_inertia(col2d::ElementPtr e, float mass, bool relative_to_cm = false);
}
