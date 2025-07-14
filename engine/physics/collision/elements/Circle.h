#pragma once

#include "core/base/UnitVector.h"
#include "core/base/Constants.h"
#include "core/base/SimpleMath.h"
#include "core/base/Transforms.h"
#include "core/math/Geometry.h"
#include "physics/collision/Tolerance.h"
#include "physics/collision/methods/CollisionInfo.h"

namespace oly::col2d
{
	namespace internal
	{
		struct CircleGlobalAccess;
	}

	struct Circle
	{
		glm::vec2 center;
		float radius;

	private:
		friend struct internal::CircleGlobalAccess;

		glm::mat2 global = 1.0f;
		glm::vec2 global_offset = {};
		glm::mat2 global_inverse = 1.0f;

	public:
		Circle(glm::vec2 center = {}, float radius = 0.0f) : center(center), radius(radius) {}

		float area() const { return glm::pi<float>() * radius * radius * glm::determinant(glm::mat2(global)); }

		static Circle fast_wrap(const math::Polygon2D& polygon);

		fpair projection_interval(const UnitVector2D& axis) const;
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

		ContactManifold deepest_manifold(const UnitVector2D& axis) const;
	};

	struct OBB;
	namespace internal
	{
		struct CircleGlobalAccess
		{
			static const glm::mat2& get_global(const Circle&);
			static glm::vec2 get_global_offset(const Circle&);
			static Circle create_affine_circle(const Circle&, const glm::mat3x2&);
			static bool has_no_global(const Circle&);
			static float radius_disparity(const Circle&);
			static float max_radius(const Circle&);
			static float min_radius(const Circle&);
			static Circle bounding_circle(const Circle&);
			static OBB bounding_obb(const Circle&);
			static glm::vec2 global_center(const Circle&);
			static glm::vec2 global_point(const Circle&, glm::vec2);
			static glm::vec2 global_direction(const Circle&, glm::vec2);
			static glm::vec2 global_normal(const Circle&, glm::vec2);
			static glm::vec2 local_point(const Circle&, glm::vec2);
			static glm::vec2 local_direction(const Circle&, glm::vec2);
		};
	}
}
