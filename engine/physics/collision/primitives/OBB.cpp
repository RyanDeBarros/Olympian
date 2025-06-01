#include "OBB.h"

#include "core/base/Errors.h"
#include "core/math/Solvers.h"
#include "core/types/Approximate.h"
#include "physics/collision/primitives/Common.h"

namespace oly::col2d
{
	OBB OBB::fast_wrap(const math::Polygon2D& polygon)
	{
		OBB obb{};

		glm::vec2 centroid = {};
		for (glm::vec2 point : polygon)
			centroid += point;
		centroid /= (float)polygon.size();

		math::solver::Eigen2x2 covariance{ .M = 0.0f };

		for (glm::vec2 point : polygon)
		{
			glm::vec2 p = point - centroid;
			covariance.M[0][0] += p.x * p.x;
			covariance.M[0][1] += p.x * p.y;
			covariance.M[1][0] += p.y * p.x;
			covariance.M[1][1] += p.y * p.y;
		}
		covariance.M /= (float)polygon.size();

		glm::vec2 eigenvectors[2];
		covariance.solve(nullptr, eigenvectors);
		glm::vec2 major_axis = eigenvectors[1];
		glm::vec2 minor_axis = eigenvectors[0];

		obb.rotation = glm::atan(major_axis.y, major_axis.x);

		float minX = std::numeric_limits<float>::max(), maxX = std::numeric_limits<float>::lowest(), minY = std::numeric_limits<float>::max(), maxY = std::numeric_limits<float>::lowest();
		for (glm::vec2 point : polygon)
		{
			glm::vec2 p = point - centroid;
			float x = glm::dot(p, major_axis);
			float y = glm::dot(p, minor_axis);

			minX = std::min(minX, x);
			maxX = std::max(maxX, x);
			minY = std::min(minY, y);
			maxY = std::max(maxY, y);
		}

		obb.center = 0.5f * glm::vec2{ minX + maxX, minY + maxY };
		obb.width = maxX - minX;
		obb.height = maxY - minY;

		return obb;
	}

	OBB OBB::wrap_axis_aligned(const math::Polygon2D& polygon, float rotation)
	{
		float max_w = std::numeric_limits<float>::lowest(), min_w = std::numeric_limits<float>::max(), max_h = std::numeric_limits<float>::lowest(), min_h = std::numeric_limits<float>::max();
		UnitVector2D axis1(rotation);
		for (glm::vec2 point : polygon)
		{
			float w = axis1.dot(point);
			min_w = std::min(min_w, w);
			max_w = std::max(max_w, w);
		}
		UnitVector2D axis2(rotation + glm::half_pi<float>());
		for (glm::vec2 point : polygon)
		{
			float h = axis2.dot(point);
			min_h = std::min(min_h, h);
			max_h = std::max(max_h, h);
		}
		return { .center = 0.5f * glm::vec2{ min_w + max_w, min_h + max_h }, .width = max_w - min_w, .height = max_h - min_h, .rotation = rotation };
	}

	std::array<glm::vec2, 4> OBB::points() const
	{
		std::array<glm::vec2, 4> points{
			glm::vec2{ -0.5f * width, -0.5f * height },
			glm::vec2{  0.5f * width, -0.5f * height },
			glm::vec2{  0.5f * width,  0.5f * height },
			glm::vec2{ -0.5f * width,  0.5f * height }
		};
		glm::mat2 rotation = get_rotation_matrix();
		for (glm::vec2& point : points)
			point = center + rotation * point;
		return points;
	}

	std::pair<float, float> OBB::projection_interval(const UnitVector2D& axis) const
	{
		if (approx(get_axis_1(), axis))
			return get_axis_1_projection_interval();
		else if (approx(get_axis_2(), axis))
			return get_axis_2_projection_interval();

		auto pts = points();
		return internal::polygon_projection_interval(pts, axis);
	}

	glm::vec2 OBB::deepest_point(const UnitVector2D& axis) const
	{
		auto pts = points();
		return internal::polygon_deepest_point(pts, axis);
	}
}
