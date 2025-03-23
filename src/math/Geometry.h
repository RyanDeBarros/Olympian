#pragma once

#include <glm/glm.hpp>

#include <vector>

namespace oly
{
	namespace math
	{
		struct Barycentric : glm::vec3
		{
			float root() const { return x; }
			float& root() { return x; }
			float prev() const { return y; }
			float& prev() { return y; }
			float next() const { return z; }
			float& next() { return z; }

			bool inside() const { return x >= 0.0f && y >= 0.0f && z >= 0.0f; }
		};

		struct Triangle2D
		{
			glm::vec2 root;
			glm::vec2 prev;
			glm::vec2 next;

			glm::vec2 dprev() const { return root - prev; }
			glm::vec2 dnext() const { return next - root; }
			float area() const;
			Barycentric barycentric(glm::vec2 point) const; // (root, prev, next)
		};

		extern float cross(glm::vec2 u, glm::vec2 v);
		
		extern std::vector<glm::ivec3> ear_clipping(glm::uint index_offset, const std::vector<glm::vec2>& points, int starting_offset = 0, int ear_cycle = 0);
	}
}
