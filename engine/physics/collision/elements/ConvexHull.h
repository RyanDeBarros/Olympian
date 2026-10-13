#pragma once

#include "core/base/SimpleMath.h"
#include "core/base/UnitVector.h"

#include "core/math/Geometry.h"

#include "physics/collision/Tolerance.h"

#include "physics/collision/elements/Common.h"

#include "physics/collision/methods/CollisionInfo.h"

namespace oly::col2d
{
	class ConvexHull
	{
	private:
		std::vector<glm::vec2> points_;
		mutable bool dirty_center_ = true;
		mutable glm::vec2 center_{};
		mutable internal::ProjectionCache proj_cache_;

	public:
        ConvexHull(std::vector<glm::vec2> points = {});

		static ConvexHull wrap(const glm::vec2* polygon, size_t count);
		static ConvexHull wrap(const math::Polygon2D& polygon);
		static ConvexHull wrap(math::Polygon2D& polygon);

        const std::vector<glm::vec2>& points() const;
        std::vector<glm::vec2>& set_points();
        size_t size() const;

		glm::vec2 center() const;
		
		fpair projection_interval(const UnitVector2D& axis) const;
		float projection_min(const UnitVector2D& axis) const;
		float projection_max(const UnitVector2D& axis) const;
		UnitVector2D edge_normal(size_t i) const;
		ContactManifold deepest_manifold(const UnitVector2D& axis) const;
	};
}
