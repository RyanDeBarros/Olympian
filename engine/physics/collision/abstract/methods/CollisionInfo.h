#pragma once

#include "external/GLM.h"

#include <vector>
#include <variant>

namespace oly::acm2d
{
	typedef bool OverlapInfo;
	
	struct GeometricInfo
	{
		OverlapInfo overlap;
		float penetration_depth;
		glm::vec2 unit_impulse;

		glm::vec2 impulse() const { return unit_impulse * penetration_depth; }
		GeometricInfo& inverted() { unit_impulse *= -1; return *this; }
	};

	struct StructuralInfo
	{
		enum
		{
			WITNESS = 0,
			CONTACT = 1,
			POINT = 0,
			LINE = 1
		};
		typedef std::pair<glm::vec2, glm::vec2> Line;
		typedef std::variant<glm::vec2, Line> ContactElement;

		GeometricInfo simple;
		std::variant<std::pair<glm::vec2, glm::vec2>, std::vector<ContactElement>> structure;
		StructuralInfo& inverted() { simple.inverted(); return *this; }
	};
}
