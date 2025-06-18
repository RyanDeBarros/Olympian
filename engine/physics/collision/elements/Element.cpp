#include "Element.h"

#include "core/base/Transforms.h"

#include <set>

namespace oly::col2d
{
	ElementParam param(const Element& e)
	{
		return std::visit([](auto&& e) -> ElementParam {
			if constexpr (is_copy_ptr<decltype(e)>)
				return e.get();
			else
				return &e;
			}, e);
	}

	CollisionResult greedy_collision(const std::vector<CollisionResult>& collisions)
	{
		CollisionResult greedy{ .overlap = true, .penetration_depth = 0.0f };
		for (const CollisionResult& collision : collisions)
		{
			if (collision.overlap && collision.penetration_depth > greedy.penetration_depth)
				greedy = collision;
		}
		return greedy;
	}

	ContactResult greedy_contact(const std::vector<ContactResult>& contacts)
	{
		ContactResult greedy{ .overlap = true };
		float greedy_depth_sqrd = 0.0f;
		for (const ContactResult& contact : contacts)
		{
			if (contact.overlap)
			{
				float depth_sqrd = math::mag_sqrd(contact.active_feature.impulse);
				if (depth_sqrd > greedy_depth_sqrd)
				{
					greedy_depth_sqrd = depth_sqrd;
					greedy = contact;
				}
			}
		}
		return greedy;
	}

	static std::set<UnitVector2D> candidate_axes(const AABB&)
	{
		return { UnitVector2D::RIGHT, UnitVector2D::UP, UnitVector2D::LEFT, UnitVector2D::DOWN };
	}

	static std::set<UnitVector2D> candidate_axes(const OBB& c)
	{
		return { c.get_major_axis(), c.get_minor_axis(), -c.get_major_axis(), -c.get_minor_axis() };
	}

	static std::set<UnitVector2D> candidate_axes(const ConvexHull& c)
	{
		std::set<UnitVector2D> axes;
		for (size_t i = 0; i < c.size(); ++i)
		{
			axes.insert(c.edge_normal(i));
			axes.insert(-c.edge_normal(i));
		}
		return axes;
	}

	static std::set<UnitVector2D> candidate_axes(const CustomKDOP& c)
	{
		std::set<UnitVector2D> axes;
		for (size_t i = 0; i < c.get_k(); ++i)
		{
			axes.insert(c.edge_normal(i));
			axes.insert(-c.edge_normal(i));
		}
		return axes;
	}

	template<size_t K>
	static std::set<UnitVector2D> candidate_axes(const KDOP<K>&)
	{
		std::set<UnitVector2D> axes;
		for (size_t i = 0; i < K; ++i)
		{
			axes.insert(KDOP<K>::uniform_axis(i));
			axes.insert(-KDOP<K>::uniform_axis(i));
		}
		return axes;
	}

	static std::set<UnitVector2D> candidate_axes(const Circle& c, ElementParam other)
	{
		glm::vec2 axis = std::visit([&c](auto&& other) {
			glm::vec2 center = internal::CircleGlobalAccess::global_center(c);
			if constexpr (visiting_class_is<decltype(*other), Circle>)
				return internal::CircleGlobalAccess::global_center(*other) - center;
			else if constexpr(visiting_class_is<decltype(*other), AABB, OBB>)
			{
				float closest_dist_sqrd = nmax<float>();
				glm::vec2 closest_point{};
				auto points = other->points();
				for (glm::vec2 pt : points)
				{
					float dist_sqrd = math::mag_sqrd(pt - center);
					if (dist_sqrd < closest_dist_sqrd)
					{
						closest_dist_sqrd = dist_sqrd;
						closest_point = pt;
						if (near_zero(closest_dist_sqrd))
							break;
					}
				}
				return closest_point - center;
			}
			else
			{
				float closest_dist_sqrd = nmax<float>();
				glm::vec2 closest_point{};
				const auto& points = other->points();
				for (glm::vec2 pt : points)
				{
					float dist_sqrd = math::mag_sqrd(pt - center);
					if (dist_sqrd < closest_dist_sqrd)
					{
						closest_dist_sqrd = dist_sqrd;
						closest_point = pt;
						if (near_zero(closest_dist_sqrd))
							break;
					}
				}
				return closest_point - center;
			}
			}, other);
		return { UnitVector2D(axis) };
	}
	
	static std::set<UnitVector2D> candidate_axes(ElementParam reference, ElementParam other)
	{
		return std::visit([&other](auto&& c) {
			if constexpr (visiting_class_is<decltype(*c), Circle>)
				return candidate_axes(*c, other);
			else
				return candidate_axes(*c);
			}, reference);
	}

	static std::set<UnitVector2D> candidate_axes(ElementParam reference, const Element* others, const size_t num_others)
	{
		return std::visit([others, num_others](auto&& c) {
			if constexpr (visiting_class_is<decltype(*c), Circle>)
			{
				std::set<UnitVector2D> axes;
				for (size_t i = 0; i < num_others; ++i)
					axes.merge(candidate_axes(*c, param(others[i])));
				return axes;
			}
			else
				return candidate_axes(*c);
			}, reference);
	}

	static std::set<UnitVector2D> candidate_axes(const Element* active_elements, const size_t num_active_elements, ElementParam static_element)
	{
		std::set<UnitVector2D> axes = candidate_axes(static_element, active_elements, num_active_elements);
		for (size_t i = 0; i < num_active_elements; ++i)
			axes.merge(candidate_axes(param(active_elements[i]), static_element));
		return axes;
	}

	static std::set<UnitVector2D> candidate_axes(const Element* active_elements, const size_t num_active_elements, const Element* static_elements, const size_t num_static_elements)
	{
		std::set<UnitVector2D> axes;
		for (size_t i = 0; i < num_static_elements; ++i)
			axes.merge(candidate_axes(active_elements, num_active_elements, param(static_elements[i])));
		return axes;
	}

	// TODO more efficient way of computing projection_max/min, such as with caching. For example, in ConvexHull cache the last axis that was queried, along with the deepest_point/min/max.
	// Therefore, the polygonal projection algorithms can start at a better initial point by comparing the previous axis queried and the current, especially since the axes visited by compound collision are ordered.

	static float projection_max(const UnitVector2D& axis, ElementParam el)
	{
		return std::visit([&axis](auto&& el) { return el->projection_max(axis); }, el);
	}

	static float projection_min(const UnitVector2D& axis, ElementParam el)
	{
		return std::visit([&axis](auto&& el) { return el->projection_min(axis); }, el);
	}

	static float separation(const UnitVector2D& axis, const Element* active_elements, const size_t num_active_elements, ElementParam static_element)
	{
		// Find what length is necessary to separate compound objects along axis
		float active_min_proj = nmax<float>();
		for (size_t i = 0; i < num_active_elements; ++i)
			active_min_proj = std::min(active_min_proj, projection_min(axis, param(active_elements[i])));
		float static_max_proj = projection_max(axis, static_element);
		return active_min_proj - static_max_proj;
	}

	static float separation(const UnitVector2D& axis, const Element* active_elements, const size_t num_active_elements, const Element* static_elements, const size_t num_static_elements)
	{
		// Find what length is necessary to separate compound objects along axis
		float active_min_proj = nmax<float>();
		for (size_t i = 0; i < num_active_elements; ++i)
			active_min_proj = std::min(active_min_proj, projection_min(axis, param(active_elements[i])));
		float static_max_proj = nmax<float>();
		for (size_t i = 0; i < num_static_elements; ++i)
			static_max_proj = std::min(static_max_proj, projection_max(axis, param(static_elements[i])));
		return active_min_proj - static_max_proj;
	}

	CollisionResult compound_collision(const Element* active_elements, const size_t num_active_elements, ElementParam static_element)
	{
		// Find candidate separating axes
		std::set<UnitVector2D> separating_axes = candidate_axes(active_elements, num_active_elements, static_element);

		// Iterate through each candidate separating axis
		// Find the laziest MTV required
		CollisionResult laziest{ .overlap = true, .penetration_depth = nmax<float>() };
		for (const UnitVector2D& axis : separating_axes)
		{
			float sep = separation(axis, active_elements, num_active_elements, static_element);
			if (sep < 0.0f)
				return { .overlap = false };
			else if (sep < laziest.penetration_depth)
			{
				laziest.penetration_depth = sep;
				laziest.unit_impulse = axis;
			}
		}
		return laziest;
	}

	CollisionResult compound_collision(const Element* active_elements, const size_t num_active_elements, const Element* static_elements, const size_t num_static_elements)
	{
		// Find all candidate MTV directions
		std::set<UnitVector2D> separating_axes = candidate_axes(active_elements, num_active_elements, static_elements, num_static_elements);

		// Iterate through each candidate separating axis
		// Find the laziest MTV required
		CollisionResult laziest{ .overlap = true, .penetration_depth = nmax<float>() };
		for (const UnitVector2D& axis : separating_axes)
		{
			float sep = separation(axis, active_elements, num_active_elements, static_elements, num_static_elements);
			if (sep < 0.0f)
				return { .overlap = false };
			else if (sep < laziest.penetration_depth)
			{
				laziest.penetration_depth = sep;
				laziest.unit_impulse = axis;
			}
		}
		return laziest;
	}

	ContactResult compound_contact(const Element* active_elements, const size_t num_active_elements, ElementParam static_element)
	{
		CollisionResult collision = compound_collision(active_elements, num_active_elements, static_element);
		ContactResult contact{ .overlap = collision.overlap };
		if (!contact.overlap)
			return contact;

		static const auto update_feature = [](glm::vec2 pt, const UnitVector2D& axis, float& max_depth, glm::vec2& contact_position) {
			float depth = axis.dot(pt);
			if (depth > max_depth)
			{
				max_depth = depth;
				contact_position = pt;
			}
			};

		contact.active_feature.impulse = collision.mtv();
		float active_depth = -nmax<float>();
		for (size_t i = 0; i < num_active_elements; ++i)
		{
			std::visit([&contact, &active_depth, axis = -collision.unit_impulse](auto&& ae) {
				if constexpr (is_copy_ptr<decltype(ae)>)
					update_feature(ae->deepest_point(axis), axis, active_depth, contact.active_feature.position);
				else
					update_feature(ae.deepest_point(axis), axis, active_depth, contact.active_feature.position);
				}, active_elements[i]);
		}

		contact.static_feature.impulse = -collision.mtv();
		contact.static_feature.position = std::visit([axis = collision.unit_impulse](auto&& se) { return se->deepest_point(axis); }, static_element);

		return contact;
	}

	ContactResult compound_contact(const Element* active_elements, const size_t num_active_elements, const Element* static_elements, const size_t num_static_elements)
	{
		CollisionResult collision = compound_collision(active_elements, num_active_elements, static_elements, num_static_elements);
		ContactResult contact{ .overlap = collision.overlap };
		if (!contact.overlap)
			return contact;

		static const auto update_feature = [](glm::vec2 pt, const UnitVector2D& axis, float& max_depth, glm::vec2& contact_position) {
			float depth = axis.dot(pt);
			if (depth > max_depth)
			{
				max_depth = depth;
				contact_position = pt;
			}
			};

		contact.active_feature.impulse = collision.mtv();
		float active_depth = -nmax<float>();
		for (size_t i = 0; i < num_active_elements; ++i)
		{
			std::visit([&contact, &active_depth, axis = -collision.unit_impulse](auto&& ae) {
				if constexpr (is_copy_ptr<decltype(ae)>)
					update_feature(ae->deepest_point(axis), axis, active_depth, contact.active_feature.position);
				else
					update_feature(ae.deepest_point(axis), axis, active_depth, contact.active_feature.position);
				}, active_elements[i]);
		}

		contact.static_feature.impulse = -collision.mtv();
		float static_depth = -nmax<float>();
		for (size_t i = 0; i < num_static_elements; ++i)
		{
			std::visit([&contact, &static_depth, axis = collision.unit_impulse](auto&& se) {
				if constexpr (is_copy_ptr<decltype(se)>)
					update_feature(se->deepest_point(axis), axis, static_depth, contact.static_feature.position);
				else
					update_feature(se.deepest_point(axis), axis, static_depth, contact.static_feature.position);
				}, static_elements[i]);
		}

		return contact;
	}

	static bool only_translation_and_scale(const glm::mat3& m)
	{
		return (near_zero(m[0][1]) && near_zero(m[1][0])) || (near_zero(m[0][0]) && near_zero(m[1][1]));
	}

	static bool orthogonal_transform(const glm::mat3& m)
	{
		return near_zero(glm::dot(m[0], m[1]));
	}

	Element internal::transform_element(const Circle& c, const glm::mat3& m)
	{
		Circle tc(c.center, c.radius);
		internal::CircleGlobalAccess::set_global(tc, m);
		return tc;
	}

	Element internal::transform_element(const AABB& c, const glm::mat3& m)
	{
		if (only_translation_and_scale(m))
		{
			// AABB
			if (near_zero(m[0][1]))
			{
				float x1 = m[0][0] * c.x1 + m[2][0];
				float x2 = m[0][0] * c.x2 + m[2][0];
				float y1 = m[1][1] * c.y1 + m[2][1];
				float y2 = m[1][1] * c.y2 + m[2][1];
				return AABB{ .x1 = std::min(x1, x2), .x2 = std::max(x1, x2), .y1 = std::min(y1, y2), .y2 = std::max(y1, y2) };
			}
			else
			{
				float x1 = m[1][0] * c.y1 + m[2][0];
				float x2 = m[1][0] * c.y2 + m[2][0];
				float y1 = m[0][1] * c.x1 + m[2][1];
				float y2 = m[0][1] * c.x2 + m[2][1];
				return AABB{ .x1 = std::min(x1, x2), .x2 = std::max(x1, x2), .y1 = std::min(y1, y2), .y2 = std::max(y1, y2) };
			}
		}
		else if (orthogonal_transform(m))
		{
			// OBB
			return OBB{ .center = m * glm::vec3(c.center(), 1.0f), .width = c.width() * glm::length(m[0]), .height = c.height() * glm::length(m[1]), .rotation = glm::atan(m[0][1], m[0][0])};
		}
		else
		{
			// CustomKDOP
			std::vector<UnitVector2D> axes(2);
			std::vector<float> minima(2);
			std::vector<float> maxima(2);

			{
				glm::vec2 normal = transform_normal(m, UnitVector2D::RIGHT);
				axes[0] = UnitVector2D(normal);
				float normalization = math::inv_magnitude(normal);
				float offset = axes[0].dot(m[2]);
				minima[0] = offset + c.x1 * normalization;
				maxima[0] = offset + c.x2 * normalization;
			}

			{
				glm::vec2 normal = transform_normal(m, UnitVector2D::UP);
				axes[1] = UnitVector2D(normal);
				float normalization = math::inv_magnitude(normal);
				float offset = axes[1].dot(m[2]);
				minima[1] = offset + c.y1 * normalization;
				maxima[1] = offset + c.y2 * normalization;
			}

			return make_copy_ptr<CustomKDOP>(std::move(axes), std::move(minima), std::move(maxima));
		}
	}

	Element internal::transform_element(const OBB& c, const glm::mat3& m)
	{
		if (orthogonal_transform(m))
		{
			float rotation = glm::atan(m[0][1], m[0][0]);
			float r = fmod((c.rotation + rotation) * glm::two_over_pi<float>(), 1.0f);
			if (near_zero(r) || approx(r, 1.0f))
			{
				// AABB
				math::Polygon2D polygon;
				polygon.reserve(4);
				for (glm::vec2 point : c.points())
					polygon.push_back(transform_point(m, point));

				return AABB::wrap(polygon.data(), polygon.size());
			}
			else
			{
				// OBB
				return OBB{ .center = transform_point(m, c.center), .width = glm::length(m[0]) * c.width, .height = glm::length(m[1]) * c.height, .rotation = c.rotation + rotation };
			}
		}
		else
		{
			// CustomKDOP
			std::vector<UnitVector2D> axes(2);
			std::vector<float> minima(2);
			std::vector<float> maxima(2);

			{
				glm::vec2 normal = transform_normal(m, c.get_major_axis());
				axes[0] = UnitVector2D(normal);
				float normalization = math::inv_magnitude(normal);
				float offset = axes[1].dot(m[2]);
				float local_offset = c.get_major_axis().dot(c.center);
				minima[0] = offset + (local_offset - 0.5f * c.width) * normalization;
				maxima[0] = offset + (local_offset + 0.5f * c.width) * normalization;
			}

			{
				glm::vec2 normal = transform_normal(m, c.get_minor_axis());
				axes[1] = UnitVector2D(normal);
				float normalization = math::inv_magnitude(normal);
				float offset = axes[1].dot(m[2]);
				float local_offset = c.get_minor_axis().dot(c.center);
				minima[1] = offset + (local_offset - 0.5f * c.height) * normalization;
				maxima[1] = offset + (local_offset + 0.5f * c.height) * normalization;
			}

			return make_copy_ptr<CustomKDOP>(std::move(axes), std::move(minima), std::move(maxima));
		}
	}

	Element internal::transform_element(const CustomKDOP& c, const glm::mat3& m)
	{
		// CustomKDOP
		// LATER check if compatible with standard KDOP axes
		std::vector<UnitVector2D> axes(c.get_k());
		std::vector<float> minima(c.get_k());
		std::vector<float> maxima(c.get_k());
		for (size_t i = 0; i < axes.size(); ++i)
		{
			glm::vec2 normal = transform_normal(m, c.edge_normal(i));
			axes[i] = UnitVector2D(normal);
			float normalization = math::inv_magnitude(normal);
			float offset = axes[i].dot(m[2]);
			minima[i] = offset + c.get_minimum(i) * normalization;
			maxima[i] = offset + c.get_maximum(i) * normalization;
		}
		return make_copy_ptr<CustomKDOP>(std::move(axes), std::move(minima), std::move(maxima));
	}

	template<size_t K>
	static Element transform_element_impl(const KDOP<K>& c, const glm::mat3& m)
	{
		bool reverse_axes = false;
		bool maintain_axes = false;
		float scale = 0.0f;
		float rotation = 0.0f;
		if (orthogonal_transform(m))
		{
			scale = glm::length(m[0]);
			float scaleY = glm::length(m[1]);
			reverse_axes = math::cross(m[0], m[1]) < 0.0f;
			rotation = glm::atan(m[0][1], m[0][0]);
			if (approx(glm::abs(scale), glm::abs(scaleY)) && near_multiple(rotation, glm::pi<float>() / K))
				maintain_axes = true;
		}

		if (maintain_axes)
		{
			// KDOP
			glm::vec2 translation = m[2];
			int rotation_axis_offset = 0;
			rotation_axis_offset = roundi(K * rotation * glm::one_over_pi<float>());
			std::array<float, K> minima;
			std::array<float, K> maxima;
			for (int i = 0; i < K; ++i)
			{
				float offset = KDOP<K>::uniform_axis(i).dot(translation);
				int og_idx = reverse_axes ? int(K) - i : i;
				og_idx = unsigned_mod(og_idx + rotation_axis_offset, int(K));
				minima[i] = c.get_minimum(og_idx) * scale + offset;
				maxima[i] = c.get_maximum(og_idx) * scale + offset;
			}
			return make_copy_ptr<KDOP<K>>(minima, maxima);
		}
		else
		{
			// CustomKDOP
			std::vector<UnitVector2D> axes(K);
			std::vector<float> minima(K);
			std::vector<float> maxima(K);
			for (size_t i = 0; i < axes.size(); ++i)
			{
				glm::vec2 normal = transform_normal(m, KDOP<K>::uniform_axis(i));
				axes[i] = UnitVector2D(normal);
				float normalization = math::inv_magnitude(normal);
				float offset = axes[i].dot(m[2]);
				minima[i] = offset + c.get_minimum(i) * normalization;
				maxima[i] = offset + c.get_maximum(i) * normalization;
			}
			return make_copy_ptr<CustomKDOP>(std::move(axes), std::move(minima), std::move(maxima));
		}
	}

	Element internal::transform_element(const KDOP3& c, const glm::mat3& m)
	{
		return transform_element_impl(c, m);
	}

	Element internal::transform_element(const KDOP4& c, const glm::mat3& m)
	{
		return transform_element_impl(c, m);
	}

	Element internal::transform_element(const KDOP5& c, const glm::mat3& m)
	{
		return transform_element_impl(c, m);
	}

	Element internal::transform_element(const KDOP6& c, const glm::mat3& m)
	{
		return transform_element_impl(c, m);
	}

	Element internal::transform_element(const KDOP7& c, const glm::mat3& m)
	{
		return transform_element_impl(c, m);
	}

	Element internal::transform_element(const KDOP8& c, const glm::mat3& m)
	{
		return transform_element_impl(c, m);
	}

	Element internal::transform_element(const ConvexHull& c, const glm::mat3& m)
	{
		ConvexHull tc;
		const std::vector<glm::vec2>& points = c.points();
		std::vector<glm::vec2>& tpoints = tc.set_points();
		tpoints.reserve(points.size());
		for (glm::vec2 p : points)
			tpoints.push_back(transform_point(m, p));
		return tc;
	}
}
