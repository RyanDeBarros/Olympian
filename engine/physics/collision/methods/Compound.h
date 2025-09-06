#pragma once

#include "physics/collision/elements/Element.h"
#include "physics/collision/methods/CompoundPerformance.h"

namespace oly::col2d
{
	extern CollisionResult compound_collision(const Element* active_elements, const size_t num_active_elements,
		const Element& static_element, const CompoundPerfParameters perf = {});
	extern CollisionResult compound_collision(const Element* active_elements, const size_t num_active_elements,
		const Element* static_elements, const size_t num_static_elements, const CompoundPerfParameters perf = {});
	extern ContactResult compound_contact(const Element* active_elements, const size_t num_active_elements,
		const Element& static_element, const CompoundPerfParameters perf = {});
	extern ContactResult compound_contact(const Element* active_elements, const size_t num_active_elements,
		const Element* static_elements, const size_t num_static_elements, const CompoundPerfParameters perf = {});
}
