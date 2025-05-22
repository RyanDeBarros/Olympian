#pragma once

#include "core/math/Geometry.h"

#include <array>

namespace oly::acm2d
{
	struct OBB
	{
		glm::vec2 center;
		float width, height;
		float rotation;

		float area() const { return width * height; }

		static OBB fast_wrap(const math::Polygon2D& polygon);
		// TODO static OBB slow_wrap(const math::Polygon2D& polygon); using rotating calipers method

		std::array<glm::vec2, 2> get_axes() const
		{
			float cos = glm::cos(rotation);
			float sin = glm::sin(rotation);
			return { glm::vec2{ cos, sin }, glm::vec2{ -sin, cos } };
		}

		glm::mat2 get_rotation_matrix() const
		{
			float cos = glm::cos(rotation);
			float sin = glm::sin(rotation);
			return { { cos, sin }, { -sin, cos } };
		}

		std::array<glm::vec2, 4> points() const;
		std::pair<float, float> projection_interval(glm::vec2 axis) const;
	};
}
