#include "Element.h"

#include "core/base/Transforms.h"
#include "core/algorithms/GoldenSectionSearch.h"
#include "core/containers/FixedVector.h"
#include "physics/collision/methods/Collide.h"

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

	static float projection_max(const UnitVector2D& axis, const ElementParam& el)
	{
		return std::visit([&axis](auto&& el) { return el->projection_max(axis); }, el);
	}

	static float projection_min(const UnitVector2D& axis, const ElementParam& el)
	{
		return std::visit([&axis](auto&& el) { return el->projection_min(axis); }, el);
	}

	//struct FullMTVResult
	//{
	//	bool overlap;
	//	UnitVector2D unit_impulse;
	//	float penetration_depth;
	//	float active_
	//};

	struct MTVPosNeg
	{
		bool overlap = false;
		UnitVector2D unit_impulse;
		float forward_depth = 0.0f;
		float backward_depth = 0.0f;

		glm::vec2 forward_mtv() const { return forward_depth * (glm::vec2)unit_impulse; }
		glm::vec2 backward_mtv() const { return backward_depth * (glm::vec2)-unit_impulse; }
	};

	static MTVPosNeg generate_mtv(const ElementParam& active_element, const ElementParam& static_element)
	{
		MTVPosNeg mtv;
		CollisionResult collision = collides(active_element, static_element);
		if (collision.overlap)
		{
			mtv.overlap = true;
			mtv.unit_impulse = collision.unit_impulse;
			mtv.forward_depth = collision.penetration_depth;
			mtv.backward_depth = projection_max(-mtv.unit_impulse, static_element) - projection_min(-mtv.unit_impulse, active_element);
		}
		return mtv;
	}

	static float separation(const UnitVector2D& axis, const ElementParam& active_element, const ElementParam& static_element, const MTVPosNeg& mtv)
	{
		if (mtv.overlap)
		{
			if (col2d::approx(axis, mtv.unit_impulse))
				return mtv.forward_depth;
			else if (col2d::approx(axis, -mtv.unit_impulse))
				return mtv.backward_depth;
			else
			{
				float separation_along_axis = projection_max(axis, static_element) - projection_min(axis, active_element);
				float dot = axis.dot(mtv.unit_impulse);
				if (near_zero(dot))
					return separation_along_axis;
				float overfitting_depth = 0.0f;
				if (dot > 0.0f)
					overfitting_depth = mtv.forward_depth * mtv.forward_depth / axis.dot(mtv.forward_mtv());
				else
					overfitting_depth = mtv.backward_depth * mtv.backward_depth / axis.dot(mtv.backward_mtv());
				// this upper bound is only possible because Elements are convex.
				return std::min(overfitting_depth, separation_along_axis);
			}
		}
		else
		{
			// LATER ? pre-emptive check if active will overlap static if travelling on some separation interval.
			// If direction is away from static, then can return 0.0. Otherwise, if overall max_sep is less than separation between objects, can return 0.0,
			// else return additional separation needed for active to overtake/pass static.
			return 0.0f;
		}
	}

	static FixedVector<MTVPosNeg> generate_mtvs(const Element* active_elements, const size_t num_active_elements, const ElementParam& static_element)
	{
		FixedVector<MTVPosNeg> mtvs(num_active_elements);
		for (size_t i = 0; i < num_active_elements; ++i)
			mtvs[i] = generate_mtv(param(active_elements[i]), static_element);
		return mtvs;
	}

	static FixedVector<MTVPosNeg> generate_mtvs(const Element* active_elements, const size_t num_active_elements, const Element* static_elements, const size_t num_static_elements)
	{
		FixedVector<MTVPosNeg> mtvs(num_active_elements * num_static_elements);
		for (size_t i = 0; i < num_active_elements; ++i)
			for (size_t j = 0; j < num_static_elements; ++j)
				mtvs[i * num_static_elements + j] = generate_mtv(param(active_elements[i]), param(static_elements[j]));
		return mtvs;
	}

	template<typename SeparationFunc>
	CollisionResult compound_collision_generic(SeparationFunc separation, const CompoundPerfParameters perf)
	{
		CollisionResult laziest{ .overlap = true, .penetration_depth = nmax<float>() };

		// coarse sweep
		int minimizing_axis = 0;
		for (size_t i = 0; i < perf.get_coarse_sweep_divisions(); ++i)
		{
			UnitVector2D axis((float)i * perf.get_two_pi_over_divisions());
			float sep = separation(axis);
			if (sep <= 0.0f)
				return { .overlap = false };
			else if (sep < laziest.penetration_depth)
			{
				laziest.penetration_depth = sep;
				laziest.unit_impulse = axis;
				minimizing_axis = (int)i;
			}
		}

		// golden-search refinement
		EarlyExitGoldenSearchResult search_result = early_exit_minimizing_golden_search([separation](float angle) { return separation(UnitVector2D(angle)); },
			(float)(minimizing_axis - 1) * perf.get_two_pi_over_divisions(), (float)(minimizing_axis + 1) * perf.get_two_pi_over_divisions(), perf.refinement_error_threshold, 0.0f);

		if (search_result.early_exited)
			return { .overlap = false };

		if (search_result.output < laziest.penetration_depth)
		{
			laziest.unit_impulse = UnitVector2D(search_result.input);
			laziest.penetration_depth = search_result.output;
		}
		return laziest;
	}

	static float separation(const UnitVector2D& axis, const Element* active_elements, const size_t num_active_elements,
		const ElementParam& static_element, const FixedVector<MTVPosNeg>& mtvs)
	{
		float max_sep = 0.0f;
		for (size_t i = 0; i < num_active_elements; ++i)
			max_sep = std::max(max_sep, separation(axis, param(active_elements[i]), static_element, mtvs[i]));
		return max_sep;
	}

	static float separation(const UnitVector2D& axis, const Element* active_elements, const size_t num_active_elements,
		const Element* static_elements, const size_t num_static_elements, const FixedVector<MTVPosNeg>& mtvs)
	{
		float max_sep = 0.0f;
		for (size_t i = 0; i < num_active_elements; ++i)
			for (size_t j = 0; j < num_static_elements; ++j)
				max_sep = std::max(max_sep, separation(axis, param(active_elements[i]), param(static_elements[j]), mtvs[i * num_static_elements + j]));
		return max_sep;
	}

	CollisionResult compound_collision(const Element* active_elements, const size_t num_active_elements,
		const ElementParam& static_element, const CompoundPerfParameters perf)
	{
		const FixedVector<MTVPosNeg> mtvs = generate_mtvs(active_elements, num_active_elements, static_element);
		return compound_collision_generic([&](UnitVector2D axis) { return separation(axis, active_elements, num_active_elements, static_element, mtvs); }, perf);
	}

	CollisionResult compound_collision(const Element* active_elements, const size_t num_active_elements,
		const Element* static_elements, const size_t num_static_elements, const CompoundPerfParameters perf)
	{
		const FixedVector<MTVPosNeg> mtvs = generate_mtvs(active_elements, num_active_elements, static_elements, num_static_elements);
		return compound_collision_generic([&](UnitVector2D axis) { return separation(axis, active_elements, num_active_elements, static_elements, num_static_elements, mtvs); }, perf);
	}

	static float separation(const UnitVector2D& axis, const Element* active_elements, const size_t num_active_elements,
		const ElementParam& static_element, size_t& most_significant_active_element, const FixedVector<MTVPosNeg>& mtvs)
	{
		float max_sep = 0.0f;
		for (size_t i = 0; i < num_active_elements; ++i)
		{
			float sep = separation(axis, param(active_elements[i]), static_element, mtvs[i]);
			if (sep > max_sep)
			{
				max_sep = sep;
				most_significant_active_element = i;
			}
		}
		return max_sep;
	}

	static float separation(const UnitVector2D& axis, const Element* active_elements, const size_t num_active_elements, const Element* static_elements,
		const size_t num_static_elements, size_t& most_significant_active_element, size_t& most_significant_static_element, const FixedVector<MTVPosNeg>& mtvs)
	{
		float max_sep = 0.0f;
		for (size_t i = 0; i < num_active_elements; ++i)
		{
			for (size_t j = 0; j < num_static_elements; ++j)
			{
				float sep = separation(axis, param(active_elements[i]), param(static_elements[j]), mtvs[i * num_static_elements + j]);
				if (sep > max_sep)
				{
					max_sep = sep;
					most_significant_active_element = i;
					most_significant_static_element = j;
				}
			}
		}
		return max_sep;
	}

	static ContactResult param_contact_result(const ElementParam& e1, const ElementParam& e2, const CollisionResult& collision)
	{
		ContactResult contact{ .overlap = collision.overlap };
		if (contact.overlap)
		{
			contact.active_contact.impulse = collision.mtv();
			contact.passive_contact.impulse = -contact.active_contact.impulse;

			ContactManifold c1 = std::visit([axis = -collision.unit_impulse](const auto& e) { return e->deepest_manifold(axis); }, e1);
			ContactManifold c2 = std::visit([axis = collision.unit_impulse](const auto& e) { return e->deepest_manifold(axis); }, e2);
			glm::vec2 p1 = {}, p2 = {};
			ContactManifold::clamp(collision.unit_impulse, c1, c2, p1, p2);

			contact.active_contact.position = p1;
			contact.passive_contact.position = p2;
		}
		return contact;
	}

	ContactResult compound_contact(const Element* active_elements, const size_t num_active_elements,
		const ElementParam& static_element, const CompoundPerfParameters perf)
	{
		const FixedVector<MTVPosNeg> mtvs = generate_mtvs(active_elements, num_active_elements, static_element);
		size_t most_significant_active_element = 0;
		CollisionResult collision = compound_collision_generic([&](UnitVector2D axis)
			{ return separation(axis, active_elements, num_active_elements, static_element, most_significant_active_element, mtvs); }, perf);
		return param_contact_result(param(active_elements[most_significant_active_element]), static_element, collision);
	}

	ContactResult compound_contact(const Element* active_elements, const size_t num_active_elements,
		const Element* static_elements, const size_t num_static_elements, const CompoundPerfParameters perf)
	{
		const FixedVector<MTVPosNeg> mtvs = generate_mtvs(active_elements, num_active_elements, static_elements, num_static_elements);
		size_t most_significant_active_element = 0, most_significant_static_element = 0;
		CollisionResult collision = compound_collision_generic([&](UnitVector2D axis)
			{ return separation(axis, active_elements, num_active_elements, static_elements, num_static_elements,
				most_significant_active_element, most_significant_static_element, mtvs); }, perf);
		return param_contact_result(param(active_elements[most_significant_active_element]), param(static_elements[most_significant_static_element]), collision);
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
		return internal::CircleGlobalAccess::create_affine_circle(c, m);
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
			// KDOP
			KDOP<2> kdop({ { c.x1, c.x2 }, { c.y1, c.y2 } });
			return internal::KDOPGlobalAccess<2>::create_affine_kdop_ptr(kdop, m);
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
			// KDOP
			KDOP<2> kdop({ { -0.5f * c.width, 0.5f * c.width}, { -0.5f * c.height, 0.5f * c.height } });
			glm::mat3 g = m * Transform2D{ .position = c.center, .rotation = c.rotation }.matrix();
			return internal::KDOPGlobalAccess<2>::create_affine_kdop_ptr(kdop, g);
		}
	}

	template<size_t K>
	static Element transform_element_impl(const KDOP<K>& c, const glm::mat3& m)
	{
		// KDOP
		return internal::KDOPGlobalAccess<K>::create_affine_kdop_ptr(c, m);
	}

	Element internal::transform_element(const KDOP2& c, const glm::mat3& m)
	{
		glm::mat3 global = m * augment(internal::KDOPGlobalAccess<2>::get_global(c), internal::KDOPGlobalAccess<2>::get_global_offset(c));
		glm::mat2 g_inv_t = glm::transpose(glm::inverse(glm::mat2(global)));
		UnitVector2D right = UnitVector2D(g_inv_t * KDOP2::uniform_axis(0));
		UnitVector2D up = UnitVector2D(g_inv_t * KDOP2::uniform_axis(1));
		if (near_zero(right.dot(up)))
		{
			math::Polygon2D points = c.points();
			for (size_t i = 0; i < 4; ++i)
				points[i] = transform_point(m, points[i]);

			if (right.near_cardinal(LINEAR_TOLERANCE))
			{
				// AABB
				return AABB{
					.x1 = min(points[0].x, points[1].x, points[2].x, points[3].x),
					.x2 = max(points[0].x, points[1].x, points[2].x, points[3].x),
					.y1 = min(points[1].y, points[1].y, points[2].y, points[3].y),
					.y2 = max(points[1].y, points[1].y, points[2].y, points[3].y)
				};
			}
			else
			{
				// OBB
				UnitVector2D rot = glm::vec2(global[0]);
				glm::mat2 inverse_rotation = rot.inverse_rotation_matrix();
				for (size_t i = 0; i < 4; ++i)
					points[i] = inverse_rotation * points[i];
				
				AABB aabb{
					.x1 = min(points[0].x, points[1].x, points[2].x, points[3].x),
					.x2 = max(points[0].x, points[1].x, points[2].x, points[3].x),
					.y1 = min(points[1].y, points[1].y, points[2].y, points[3].y),
					.y2 = max(points[1].y, points[1].y, points[2].y, points[3].y)
				};

				return OBB{ .center = rot.rotation_matrix() * aabb.center(), .width = aabb.width(), .height = aabb.height(), .rotation = rot.rotation() };
			}
		}
		else
			return transform_element_impl(c, m);
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
