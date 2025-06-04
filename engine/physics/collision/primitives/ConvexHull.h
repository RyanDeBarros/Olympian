#pragma once

#include "core/base/UnitVector.h"
#include "core/math/Geometry.h"

namespace oly::col2d
{
	class ConvexHull
	{
	private:
		std::vector<glm::vec2> _points;
		mutable bool dirty_center = true;
		mutable glm::vec2 _center{};

	public:
		ConvexHull(const std::vector<glm::vec2>& points = {}) : _points(points) {}
		ConvexHull(std::vector<glm::vec2>&& points) : _points(std::move(points)) {}

		const std::vector<glm::vec2> points() const { return _points; }
		std::vector<glm::vec2>& set_points() { dirty_center = true; return _points; }

		static ConvexHull wrap(const math::Polygon2D& polygon);
		static ConvexHull wrap(math::Polygon2D& polygon);
		
		glm::vec2 center() const;
		
		std::pair<float, float> projection_interval(const UnitVector2D& axis) const;
		UnitVector2D edge_normal(size_t i) const;
		glm::vec2 deepest_point(const UnitVector2D& axis) const;
	};
}
