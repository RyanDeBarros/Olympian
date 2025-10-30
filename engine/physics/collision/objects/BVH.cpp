#include "BVH.h"

namespace oly::col2d
{
	namespace internal
	{
		AABB Wrap<AABB>::operator()(const Element* elements, size_t count) const
		{
			AABB c = AABB::DEFAULT;
			for (size_t i = 0; i < count; ++i)
			{
				AABB sub = operator()(elements[i]);
				c.x1 = std::min(c.x1, sub.x1);
				c.x2 = std::max(c.x2, sub.x2);
				c.y1 = std::min(c.y1, sub.y1);
				c.y2 = std::max(c.y2, sub.y2);
			}
			return c;
		}

		AABB Wrap<AABB>::operator()(const Element& element) const
		{
			return element.aabb_wrap();
		}

		static glm::vec2 compute_centroid_sum(const Element& element)
		{
			return element.variant().visit([](const auto& element) -> glm::vec2 {
				if constexpr (visiting_class_is<decltype(*element), Circle>)
					return element->center;
				else
				{
					glm::vec2 centroid = {};
					for (glm::vec2 p : element->points())
						centroid += p;
					return centroid;
				}
				});
		}

		static size_t compute_centroid_point_count(const Element& element)
		{
			return element.variant().visit([](const auto& element) -> size_t {
				if constexpr (visiting_class_is<decltype(*element), Circle>)
					return 1;
				else if constexpr (visiting_class_is<decltype(*element), AABB, OBB>)
					return 4;
				else
					return element->points().size();
				});
		}

		static glm::mat2 compute_covariance(const Element& element, glm::vec2 centroid)
		{
			return element.variant().visit([centroid](const auto& element) -> glm::mat2 {
				if constexpr (visiting_class_is<decltype(*element), Circle>)
				{
					glm::vec2 p = element->center - centroid;
					return { { p.x * p.x, p.x * p.y }, { p.y * p.x, p.y * p.y } };
				}
				else
				{
					glm::mat2 covariance = 0.0f;
					for (glm::vec2 point : element->points())
					{
						glm::vec2 p = point - centroid;
						covariance[0][0] += p.x * p.x;
						covariance[0][1] += p.x * p.y;
						covariance[1][0] += p.y * p.x;
						covariance[1][1] += p.y * p.y;
					}
					return covariance;
				}
				});
		}

		static AABB compute_obb_bounds(const Element& element, const UnitVector2D& major_axis, const UnitVector2D& minor_axis)
		{
			return element.variant().visit([&major_axis, &minor_axis](const auto& element) -> AABB {
				if constexpr (visiting_class_is<decltype(*element), Circle>)
					return { .x1 = (-major_axis).dot(element->deepest_manifold(-major_axis).pt()), .x2 = major_axis.dot(element->deepest_manifold(major_axis).pt()),
							 .y1 = (-minor_axis).dot(element->deepest_manifold(-minor_axis).pt()), .y2 = minor_axis.dot(element->deepest_manifold(minor_axis).pt()) };
				else
				{
					AABB bounds = AABB::DEFAULT;
					for (glm::vec2 point : element->points())
					{
						float x = major_axis.dot(point);
						float y = minor_axis.dot(point);

						bounds.x1 = std::min(bounds.x1, x);
						bounds.x2 = std::max(bounds.x2, x);
						bounds.y1 = std::min(bounds.y1, y);
						bounds.y2 = std::max(bounds.y2, y);
					}
					return bounds;
				}
				});
		}

		OBB Wrap<OBB>::operator()(const Element* elements, size_t count) const
		{
			glm::vec2 centroid = {};
			size_t point_count = 0;
			for (size_t i = 0; i < count; ++i)
			{
				centroid += compute_centroid_sum(elements[i]);
				point_count += compute_centroid_point_count(elements[i]);
			}
			centroid /= (float)point_count;

			math::solver::Eigen2x2 covariance{ .M = 0.0f };

			for (size_t i = 0; i < count; ++i)
				covariance.M += compute_covariance(elements[i], centroid);
			covariance.M /= (float)point_count;

			glm::vec2 eigenvectors[2];
			covariance.solve(nullptr, eigenvectors);
			UnitVector2D major_axis = eigenvectors[1];
			UnitVector2D minor_axis = major_axis.get_quarter_turn();

			AABB bounds = AABB::DEFAULT;
			for (size_t i = 0; i < count; ++i)
			{
				AABB sub = compute_obb_bounds(elements[i], major_axis, minor_axis);
				bounds.x1 = std::min(bounds.x1, sub.x1);
				bounds.x2 = std::max(bounds.x2, sub.x2);
				bounds.y1 = std::min(bounds.y1, sub.y1);
				bounds.y2 = std::max(bounds.y2, sub.y2);
			}

			return { .center = major_axis.rotation_matrix() * bounds.center(), .width = bounds.width(), .height = bounds.height(), .rotation = major_axis.rotation()};
		}

		Circle Wrap<Circle>::operator()(const Element* elements, size_t count) const
		{
			std::vector<glm::vec2> csums;
			glm::vec2 centroid = {};
			size_t point_count = 0;
			for (size_t i = 0; i < count; ++i)
			{
				csums.push_back(compute_centroid_sum(elements[i]));
				centroid += csums[i];
				point_count += compute_centroid_point_count(elements[i]);
			}
			centroid /= (float)point_count;
			
			float radius = 0.0f;
			for (glm::vec2 p : csums)
				radius = std::max(radius, math::mag_sqrd(p - centroid));
			radius = glm::sqrt(radius);
			return Circle(centroid, radius);
		}

		static void add_point_cloud(const Element& element, std::vector<glm::vec2>& point_cloud)
		{
			element.variant().visit([&point_cloud](const auto& e) -> void {
				if constexpr (visiting_class_is<decltype(*e), Circle>)
				{
					size_t start = point_cloud.size();
					static const auto& enclosure = Wrap<ConvexHull>::CIRCLE_POLYGON_ENCLOSURE;
					point_cloud.resize(point_cloud.size() + enclosure.get_degree());
					for (size_t i = 0; i < enclosure.get_degree(); ++i)
						point_cloud[start + i] = CircleGlobalAccess::global_point(*e, enclosure.get_point(*e, i));
				}
				else if constexpr (visiting_class_is<decltype(*e), AABB, OBB>)
				{
					auto points = e->points();
					point_cloud.insert(point_cloud.end(), points.begin(), points.end());
				}
				else
				{
					const auto& points = e->points();
					point_cloud.insert(point_cloud.end(), points.begin(), points.end());
				}
				});
		}

		ConvexHull Wrap<ConvexHull>::operator()(const Element* elements, size_t count) const
		{
			std::vector<glm::vec2> point_cloud;
			for (size_t i = 0; i < count; ++i)
				add_point_cloud(elements[i], point_cloud);
			return ConvexHull::wrap(point_cloud);
		}

		glm::vec2 midpoint(const Element& element)
		{
			return element.variant().visit([](const auto& element) -> glm::vec2 {
				if constexpr (visiting_class_is<decltype(*element), Circle>)
					return internal::CircleGlobalAccess::global_center(*element);
				else if constexpr (visiting_class_is<decltype(*element), OBB>)
					return element->center;
				else
					return element->center();
				});
		}
	}
}
