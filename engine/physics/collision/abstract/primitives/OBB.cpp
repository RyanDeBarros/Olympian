#include "OBB.h"

#include "core/base/Errors.h"
#include "core/math/Solvers.h"

namespace oly::acm2d
{
	OBB OBB::fast_wrap(const math::Polygon2D& polygon)
	{
		OBB obb{};

		glm::vec2 centroid = {};
		for (glm::vec2 point : polygon.points)
			centroid += point;
		centroid /= (float)polygon.points.size();

		math::solver::Eigen2x2 covariance{ .M = 0.0f };

		for (glm::vec2 point : polygon.points)
		{
			glm::vec2 p = point - centroid;
			covariance.M[0][0] += p.x * p.x;
			covariance.M[0][1] += p.x * p.y;
			covariance.M[1][0] += p.y * p.x;
			covariance.M[1][1] += p.y * p.y;
		}
		covariance.M /= (float)polygon.points.size();

		glm::vec2 eigenvectors[2];
		covariance.solve(nullptr, eigenvectors);
		glm::vec2 major_axis = eigenvectors[1];
		glm::vec2 minor_axis = eigenvectors[0];

		obb.rotation = glm::atan(major_axis.y, major_axis.x);

		float minX = FLT_MAX, maxX = -FLT_MAX, minY = FLT_MAX, maxY = -FLT_MAX;
		for (glm::vec2 point : polygon.points)
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
}
