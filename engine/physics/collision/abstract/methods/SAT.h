#pragma once

#include "physics/collision/abstract/methods/CollisionInfo.h"
#include "physics/collision/abstract/primitives/Circle.h"

namespace oly::acm2d::sat
{
	extern OverlapInfo overlap(const std::vector<glm::vec2>& c1, const std::vector<glm::vec2>& c2);
	extern GeometricInfo geometric_collision(const std::vector<glm::vec2>& c1, const std::vector<glm::vec2>& c2);

	extern OverlapInfo overlap(const Circle& c1, const std::vector<glm::vec2>& c2);
	extern GeometricInfo geometric_collision(const Circle& c1, const std::vector<glm::vec2>& c2);
	inline OverlapInfo overlap(const std::vector<glm::vec2>& c1, const Circle& c2) { return overlap(c2, c1); }
	inline GeometricInfo geometric_collision(const std::vector<glm::vec2>& c1, const Circle& c2) { return geometric_collision(c2, c1).inverted(); }
}
