#include "BVH.h"

namespace oly::col2d
{
	namespace internal
	{
		static AABB compute_aabb(const Primitive& primitive)
		{
			return std::visit([](auto&& primitive) {
				if constexpr (std::is_same_v<std::decay_t<decltype(primitive)>, AABB>)
					return primitive;
				else if constexpr (std::is_same_v<std::decay_t<decltype(primitive)>, Circle>)
					return AABB{ .x1 = primitive.deepest_point(UnitVector2D::LEFT).x, .x2 = primitive.deepest_point(UnitVector2D::RIGHT).x,
								 .y1 = primitive.deepest_point(UnitVector2D::DOWN).y, .y2 = primitive.deepest_point(UnitVector2D::UP).y };
				else
				{
					AABB bounds = AABB::DEFAULT;
					for (glm::vec2 p : primitive.points())
					{
						bounds.x1 = std::min(bounds.x1, p.x);
						bounds.x2 = std::max(bounds.x2, p.x);
						bounds.y1 = std::min(bounds.y1, p.y);
						bounds.y2 = std::max(bounds.y2, p.y);
					}
					return bounds;
				}
				}, primitive);
		}

		AABB Wrap<AABB>::operator()(const Primitive* primitives, size_t count) const
		{
			AABB c = AABB::DEFAULT;
			for (size_t i = 0; i < count; ++i)
			{
				AABB sub = compute_aabb(primitives[i]);
				c.x1 = std::min(c.x1, sub.x1);
				c.x2 = std::max(c.x2, sub.x2);
				c.y1 = std::min(c.y1, sub.y1);
				c.y2 = std::max(c.y2, sub.y2);
			}
			return c;
		}

		static glm::vec2 compute_centroid_sum(const Primitive& primitive)
		{
			return std::visit([](auto&& primitive) -> glm::vec2 {
				if constexpr (std::is_same_v<std::decay_t<decltype(primitive)>, Circle>)
					return primitive.center;
				else
				{
					glm::vec2 centroid = {};
					for (glm::vec2 p : primitive.points())
						centroid += p;
					return centroid;
				}
				}, primitive);
		}

		static size_t compute_centroid_point_count(const Primitive& primitive)
		{
			return std::visit([](auto&& primitive) -> size_t {
				if constexpr (std::is_same_v<std::decay_t<decltype(primitive)>, Circle>)
					return 1;
				else if constexpr (std::is_same_v<std::decay_t<decltype(primitive)>, AABB>)
					return 4;
				else if constexpr (std::is_same_v<std::decay_t<decltype(primitive)>, OBB>)
					return 4;
				else
					return primitive.points().size();
				}, primitive);
		}

		static glm::mat2 compute_covariance(const Primitive& primitive, glm::vec2 centroid)
		{
			return std::visit([centroid](auto&& primitive) -> glm::mat2 {
				if constexpr (std::is_same_v<std::decay_t<decltype(primitive)>, Circle>)
				{
					glm::vec2 p = primitive.center - centroid;
					return { { p.x * p.x, p.x * p.y }, { p.y * p.x, p.y * p.y } };
				}
				else
				{
					glm::mat2 covariance = 0.0f;
					for (glm::vec2 point : primitive.points())
					{
						glm::vec2 p = point - centroid;
						covariance[0][0] += p.x * p.x;
						covariance[0][1] += p.x * p.y;
						covariance[1][0] += p.y * p.x;
						covariance[1][1] += p.y * p.y;
					}
					return covariance;
				}
				}, primitive);
		}

		static AABB compute_obb_bounds(const Primitive& primitive, glm::vec2 centroid, const UnitVector2D& major_axis, const UnitVector2D& minor_axis)
		{
			return std::visit([&major_axis, &minor_axis, centroid](auto&& primitive) -> AABB {
				if constexpr (std::is_same_v<std::decay_t<decltype(primitive)>, Circle>)
					return { .x1 = (-major_axis).dot(primitive.deepest_point(-major_axis)), .x2 = major_axis.dot(primitive.deepest_point(major_axis)),
							 .y1 = (-minor_axis).dot(primitive.deepest_point(-minor_axis)), .y2 = minor_axis.dot(primitive.deepest_point(minor_axis)) };
				else
				{
					AABB bounds = AABB::DEFAULT;
					for (glm::vec2 point : primitive.points())
					{
						glm::vec2 p = point - centroid;
						float x = major_axis.dot(p);
						float y = minor_axis.dot(p);

						bounds.x1 = std::min(bounds.x1, x);
						bounds.x2 = std::max(bounds.x2, x);
						bounds.y1 = std::min(bounds.y1, y);
						bounds.y2 = std::max(bounds.y2, y);
					}
					return bounds;
				}
				}, primitive);
		}

		OBB Wrap<OBB>::operator()(const Primitive* primitives, size_t count) const
		{
			OBB obb{};

			glm::vec2 centroid = {};
			size_t point_count = 0;
			for (size_t i = 0; i < count; ++i)
			{
				centroid += compute_centroid_sum(primitives[i]);
				point_count += compute_centroid_point_count(primitives[i]);
			}
			centroid /= (float)point_count;

			math::solver::Eigen2x2 covariance{ .M = 0.0f };

			for (size_t i = 0; i < count; ++i)
				covariance.M += compute_covariance(primitives[i], centroid);
			covariance.M /= (float)point_count;

			glm::vec2 eigenvectors[2];
			covariance.solve(nullptr, eigenvectors);
			UnitVector2D major_axis = eigenvectors[1];
			UnitVector2D minor_axis = eigenvectors[0];

			obb.rotation = major_axis.rotation();

			AABB bounds = AABB::DEFAULT;
			for (size_t i = 0; i < count; ++i)
			{
				AABB sub = compute_obb_bounds(primitives[i], centroid, major_axis, minor_axis);
				bounds.x1 = std::min(bounds.x1, sub.x1);
				bounds.x2 = std::max(bounds.x2, sub.x2);
				bounds.y1 = std::min(bounds.y1, sub.y1);
				bounds.y2 = std::max(bounds.y2, sub.y2);
			}

			return { .center = bounds.center(), .width = bounds.x2 - bounds.x1, .height = bounds.y2 - bounds.y1 };
		}
	}

	namespace heuristics
	{
		glm::vec2 midpoint(const Primitive& primitive)
		{
			return std::visit([](auto&& primitive) -> glm::vec2 {
				using PT = std::decay_t<decltype(primitive)>;
				if constexpr (std::is_same_v<PT, Circle>)
					return transform_point(internal::CircleGlobalAccess::get_global(primitive), primitive.center);
				else if constexpr (std::is_same_v<PT, OBB>)
					return primitive.center;
				else
					return primitive.center();
				}, primitive);
		}
	}
}
