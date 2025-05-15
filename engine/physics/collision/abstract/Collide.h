#pragma once

#include "physics/collision/abstract/ConvexHull.h"
#include "physics/collision/abstract/AABB.h"
#include "physics/collision/abstract/Circle.h"
#include "physics/collision/abstract/OBB.h"
#include "physics/collision/abstract/KDOP.h"

namespace oly::acm2d
{
	struct CollisionInfo
	{
		bool colliding;
		float depth;
		glm::vec2 normal;
		std::vector<glm::vec2> contact_points;
		std::pair<glm::vec2, glm::vec2> witness_points;
	};

	extern bool collide(const Circle& c1, const Circle& c2, CollisionInfo* info = nullptr);
}
