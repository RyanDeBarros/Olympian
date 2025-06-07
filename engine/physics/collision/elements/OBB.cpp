#include "OBB.h"

#include "core/base/Errors.h"
#include "core/base/Transforms.h"
#include "core/math/Solvers.h"
#include "core/types/Approximate.h"
#include "physics/collision/elements/Common.h"
#include "physics/collision/elements/ConvexHull.h"

namespace oly::col2d
{
	OBB OBB::fast_wrap(const glm::vec2* polygon, size_t count)
	{
		OBB obb{};

		glm::vec2 centroid = {};
		for (size_t i = 0; i < count; ++i)
			centroid += polygon[i];
		centroid /= (float)count;

		math::solver::Eigen2x2 covariance{ .M = 0.0f };

		for (size_t i = 0; i < count; ++i)
		{
			glm::vec2 p = polygon[i] - centroid;
			covariance.M[0][0] += p.x * p.x;
			covariance.M[0][1] += p.x * p.y;
			covariance.M[1][0] += p.y * p.x;
			covariance.M[1][1] += p.y * p.y;
		}
		covariance.M /= (float)count;

		glm::vec2 eigenvectors[2];
		covariance.solve(nullptr, eigenvectors);
		UnitVector2D major_axis = eigenvectors[1];
		UnitVector2D minor_axis = eigenvectors[0];

		obb.rotation = major_axis.rotation();

		AABB bounds = AABB::DEFAULT;
		for (size_t i = 0; i < count; ++i)
		{
			glm::vec2 p = polygon[i] - centroid;
			float x = major_axis.dot(p);
			float y = minor_axis.dot(p);

			bounds.x1 = std::min(bounds.x1, x);
			bounds.x2 = std::max(bounds.x2, x);
			bounds.y1 = std::min(bounds.y1, y);
			bounds.y2 = std::max(bounds.y2, y);
		}

		return { .center = bounds.center(), .width = bounds.width(), .height = bounds.height() };
	}

	OBB OBB::wrap_axis_aligned(const glm::vec2* polygon, size_t count, float rotation)
	{
		AABB bounds = AABB::DEFAULT;
		UnitVector2D axis1(rotation);
		for (size_t i = 0; i < count; ++i)
		{
			float w = axis1.dot(polygon[i]);
			bounds.x1 = std::min(bounds.x1, w);
			bounds.x2 = std::max(bounds.x2, w);
		}
		UnitVector2D axis2(rotation + glm::half_pi<float>());
		for (size_t i = 0; i < count; ++i)
		{
			float h = axis2.dot(polygon[i]);
			bounds.y1 = std::min(bounds.y1, h);
			bounds.y2 = std::max(bounds.y2, h);
		}
		
		return { .center = bounds.center(), .width = bounds.width(), .height = bounds.height(), .rotation = rotation};
	}

	OBB OBB::slow_wrap(const glm::vec2* polygon, size_t count)
	{
		// rotating calipers method

		ConvexHull hull = ConvexHull::wrap(polygon, count);
		size_t n = hull.size();
		float min_area = nmax<float>();
		OBB obb{};

		for (size_t i = 0; i < n; ++i)
		{
			UnitVector2D major_axis = hull.points()[(i + 1) % n] - hull.points()[i];
			UnitVector2D minor_axis = major_axis.get_quarter_turn();

			AABB bounds = AABB::DEFAULT;
			for (size_t j = 0; j < n; ++j)
			{
				float w = major_axis.dot(hull.points()[j]);
				float h = minor_axis.dot(hull.points()[j]);
				bounds.x1 = std::min(bounds.x1, w);
				bounds.x2 = std::max(bounds.x2, w);
				bounds.y1 = std::min(bounds.y1, h);
				bounds.y2 = std::max(bounds.y2, h);
			}

			float area = bounds.area();
			if (area < min_area)
			{
				min_area = area;
				obb.rotation = major_axis.rotation();
				obb.center = major_axis.rotation_matrix() * bounds.center();
				obb.width = bounds.width();
				obb.height = bounds.height();
			}
		}
		return obb;
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
		if (near_zero(math::cross(get_major_axis(), axis)))
		{
			float m = axis.dot(center);
			return { m - 0.5f * width, m + 0.5f * width };
		}
		else if (near_zero(math::cross(get_minor_axis(), axis)))
		{
			float m = axis.dot(center);
			return { m - 0.5f * height, m + 0.5f * height };
		}

		return internal::polygon_projection_interval(points(), axis);
	}

	float OBB::projection_min(const UnitVector2D& axis) const
	{
		if (near_zero(math::cross(get_major_axis(), axis)))
			return axis.dot(center) - 0.5f * width;
		else if (near_zero(math::cross(get_minor_axis(), axis)))
			return axis.dot(center) - 0.5f * height;

		return internal::polygon_projection_min(points(), axis);
	}

	float OBB::projection_max(const UnitVector2D& axis) const
	{
		if (near_zero(math::cross(get_major_axis(), axis)))
			return axis.dot(center) + 0.5f * width;
		else if (near_zero(math::cross(get_minor_axis(), axis)))
			return axis.dot(center) + 0.5f * height;

		return internal::polygon_projection_max(points(), axis);
	}

	glm::vec2 OBB::deepest_point(const UnitVector2D& axis) const
	{
		return internal::polygon_deepest_point(points(), axis);
	}
}
