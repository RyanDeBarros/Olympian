#include "OBB.h"

#include "core/base/Errors.h"

namespace oly::acm2d
{
	OBB OBB::fast_wrap(const math::Polygon2D& polygon, unsigned int quadrant_steps, unsigned int iterations)
	{
		if (quadrant_steps <= 1 || iterations == 0)
			throw Error(ErrorCode::BAD_COLLISION_PRIMITIVE);

		// find minimizing OBB for a given angle
		static const auto obb_compute = [](const math::Polygon2D& polygon, float angle) {
			float max_w = -FLT_MAX, min_w = FLT_MAX, max_h = -FLT_MAX, min_h = FLT_MAX;

			glm::vec2 axis_w = math::dir_vector(angle);
			glm::vec2 axis_h = math::dir_vector(angle + glm::half_pi<float>());

			for (glm::vec2 point : polygon.points)
			{
				float w = math::projection_distance(point, axis_w);
				float h = math::projection_distance(point, axis_h);
				max_w = std::max(max_w, w);
				min_w = std::min(min_w, w);
				max_h = std::max(max_h, h);
				min_h = std::min(min_h, h);
			}

			OBB c{};
			c.width = max_w - min_w;
			c.height = max_h - min_h;
			c.center = 0.5f * (max_w + min_w) * axis_w + 0.5f * (max_h + min_h) * axis_h;
			c.rotation = angle;
			return c;
			};

		// find minimizing OBB within a sector
		static const auto iteration = [](const math::Polygon2D& polygon, float min_angle, float max_angle, unsigned int quadrant_steps, float& new_min_angle, float& new_max_angle) {
			float min_area = FLT_MAX;
			OBB fit_obb{};

			float delta_angle = (max_angle - min_angle) / (quadrant_steps - 1);
			for (unsigned int i = 0; i < quadrant_steps; ++i)
			{
				OBB c = obb_compute(polygon, min_angle + i * delta_angle);
				float area = c.area();
				if (area < min_area)
				{
					min_area = area;
					fit_obb = c;
					new_min_angle = min_angle + (i - 1) * delta_angle;
					new_max_angle = min_angle + (i + 1) * delta_angle;
				}
			}

			return fit_obb;
			};

		float min_angle = 0.0f;
		float max_angle = glm::half_pi<float>();
		OBB c{};
		for (unsigned int i = 0; i < iterations; ++i)
			c = iteration(polygon, min_angle, max_angle, quadrant_steps, min_angle, max_angle);
		return c;
	}
}
