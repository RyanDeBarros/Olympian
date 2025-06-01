#pragma once

#include "core/math/Shapes.h"

namespace oly::math
{
	namespace coordinates
	{
		inline glm::vec2 to_polar(glm::vec2 cartesian) { return { glm::length(cartesian), glm::atan(cartesian.y, cartesian.x) }; }
		inline glm::vec2 to_cartesian(glm::vec2 polar) { return polar.x * glm::vec2{ glm::cos(polar.y), glm::sin(polar.y) }; }
		inline glm::vec3 to_spherical(glm::vec3 cartesian) {
			float r = glm::length(cartesian);
			return r != 0.0f ? glm::vec3{ r, glm::atan(cartesian.y, cartesian.x), glm::acos(cartesian.z / r) } : glm::vec3{ 0.0f, 0.0f, 0.0f };
		}
		inline glm::vec3 to_cartesian(glm::vec3 spherical) { float sphi = glm::sin(spherical.z); return spherical.x * glm::vec3{ glm::cos(spherical.y) * sphi, glm::sin(spherical.y) * sphi, glm::cosh(spherical.z) }; }
	}

	struct Barycentric : glm::vec3
	{
		using glm::vec3::vec;

		Barycentric(const Triangle2D& triangle, glm::vec2 point);

		float root() const { return x; }
		float& root() { return x; }
		float prev() const { return y; }
		float& prev() { return y; }
		float next() const { return z; }
		float& next() { return z; }

		bool inside() const { return x >= 0.0f && y >= 0.0f && z >= 0.0f; }
		glm::vec2 point(const Triangle2D& triangle) const { return x * triangle.root + y * triangle.prev + z * triangle.next; }
		glm::vec2 point(glm::vec2 a, glm::vec2 b, glm::vec2 c) const { return x * a + y * b + z * c; }
	};
}
