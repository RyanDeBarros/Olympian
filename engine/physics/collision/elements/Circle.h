#pragma once

#include "core/base/UnitVector.h"
#include "core/math/Geometry.h"
#include "core/base/Constants.h"
#include "physics/collision/Tolerance.h"

namespace oly::col2d
{
	struct Circle;
	struct OBB;
	namespace internal
	{
		struct CircleGlobalAccess
		{
			static const glm::mat3x2& get_global(const Circle&);
			static const glm::mat3x2& get_ginv(const Circle&);
			static void set_global(Circle&, const glm::mat3x2&);
			static void set_ginv(Circle&, const glm::mat3x2&);
			static inline const glm::mat3x2 DEFAULT = { glm::vec2{ 1.0f, 0.0f }, glm::vec2{ 0.0f, 1.0f }, glm::vec2{ 0.0f, 0.0f } };
			static bool has_no_global(const Circle&);
			static float radius_disparity(const Circle&);
			static float max_radius(const Circle&);
			static float min_radius(const Circle&);
			static Circle bounding_circle(const Circle&);
			static OBB bounding_obb(const Circle&);
		};
	}

	struct Circle
	{
		glm::vec2 center;
		float radius;

	private:
		friend struct internal::CircleGlobalAccess;

		glm::mat3x2 global = internal::CircleGlobalAccess::DEFAULT;
		glm::mat3x2 ginv = internal::CircleGlobalAccess::DEFAULT;

	public:
		Circle(glm::vec2 center = {}, float radius = 0.0f) : center(center), radius(radius) {}

		float area() const { return glm::pi<float>() * radius * radius * glm::determinant(glm::mat2(global)); }

		static Circle fast_wrap(const math::Polygon2D& polygon);

		std::pair<float, float> projection_interval(const UnitVector2D& axis) const;
		float projection_min(const UnitVector2D& axis) const;
		float projection_max(const UnitVector2D& axis) const;

		template<typename Polygon>
		glm::vec2 closest_point(const Polygon& polygon) const
		{
			float closest_dist_sqrd = nmax<float>();
			glm::vec2 cp{};
			for (glm::vec2 point : polygon)
			{
				float dist_sqrd = math::mag_sqrd(center - point);
				if (dist_sqrd < closest_dist_sqrd)
				{
					closest_dist_sqrd = dist_sqrd;
					cp = point;
				}
			}
			return cp;
		}

		glm::vec2 deepest_point(const UnitVector2D& axis) const;
	};
}
