#include "Compound.h"

#include "core/algorithms/GoldenSectionSearch.h"
#include "core/containers/FixedVector.h"
#include "physics/collision/methods/Collide.h"

namespace oly::col2d
{
	struct MTVPosNeg
	{
		bool overlap = false;
		UnitVector2D unit_impulse;
		float forward_depth = 0.0f;
		float backward_depth = 0.0f;

		glm::vec2 forward_mtv() const { return forward_depth * (glm::vec2)unit_impulse; }
		glm::vec2 backward_mtv() const { return backward_depth * (glm::vec2)-unit_impulse; }
	};

	static MTVPosNeg generate_mtv(const Element& active_element, const Element& static_element)
	{
		MTVPosNeg mtv;
		CollisionResult collision = collides(active_element, static_element);
		if (collision.overlap)
		{
			mtv.overlap = true;
			mtv.unit_impulse = collision.unit_impulse;
			mtv.forward_depth = collision.penetration_depth;
			mtv.backward_depth = static_element.projection_max(-mtv.unit_impulse) - active_element.projection_min(-mtv.unit_impulse);
		}
		return mtv;
	}

	static float separation(const UnitVector2D& axis, const Element& active_element, const Element& static_element, const MTVPosNeg& mtv)
	{
		if (mtv.overlap)
		{
			if (col2d::approx(axis, mtv.unit_impulse))
				return mtv.forward_depth;
			else if (col2d::approx(axis, -mtv.unit_impulse))
				return mtv.backward_depth;
			else
			{
				float separation_along_axis = static_element.projection_max(axis) - active_element.projection_min(axis);
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
			// TODO v7 ? pre-emptive check if active will overlap static if travelling on some separation interval.
			// If direction is away from static, then can return 0.0. Otherwise, if overall max_sep is less than separation between objects, can return 0.0,
			// else return additional separation needed for active to overtake/pass static.
			return 0.0f;
		}
	}

	static FixedVector<MTVPosNeg> generate_mtvs(const Element* active_elements, const size_t num_active_elements, const Element& static_element)
	{
		FixedVector<MTVPosNeg> mtvs(num_active_elements);
		for (size_t i = 0; i < num_active_elements; ++i)
			mtvs[i] = generate_mtv(active_elements[i], static_element);
		return mtvs;
	}

	static FixedVector<MTVPosNeg> generate_mtvs(const Element* active_elements, const size_t num_active_elements, const Element* static_elements, const size_t num_static_elements)
	{
		FixedVector<MTVPosNeg> mtvs(num_active_elements * num_static_elements);
		for (size_t i = 0; i < num_active_elements; ++i)
			for (size_t j = 0; j < num_static_elements; ++j)
				mtvs[i * num_static_elements + j] = generate_mtv(active_elements[i], static_elements[j]);
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
			if (sep < 0.0f)
				return { .overlap = false };
			else if (sep < laziest.penetration_depth)
			{
				laziest.penetration_depth = sep;
				laziest.unit_impulse = axis;
				minimizing_axis = (int)i;
			}
		}

		// golden-search refinement
		algo::EarlyExitGoldenSearchResult search_result = algo::early_exit_minimizing_golden_search([separation](float angle) { return separation(UnitVector2D(angle)); },
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
		const Element& static_element, const FixedVector<MTVPosNeg>& mtvs)
	{
		float max_sep = 0.0f;
		for (size_t i = 0; i < num_active_elements; ++i)
			max_sep = std::max(max_sep, separation(axis, active_elements[i], static_element, mtvs[i]));
		return max_sep;
	}

	static float separation(const UnitVector2D& axis, const Element* active_elements, const size_t num_active_elements,
		const Element* static_elements, const size_t num_static_elements, const FixedVector<MTVPosNeg>& mtvs)
	{
		float max_sep = 0.0f;
		for (size_t i = 0; i < num_active_elements; ++i)
			for (size_t j = 0; j < num_static_elements; ++j)
				max_sep = std::max(max_sep, separation(axis, active_elements[i], static_elements[j], mtvs[i * num_static_elements + j]));
		return max_sep;
	}

	CollisionResult compound_collision(const Element* active_elements, const size_t num_active_elements,
		const Element& static_element, const CompoundPerfParameters perf)
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
		const Element& static_element, size_t& most_significant_active_element, const FixedVector<MTVPosNeg>& mtvs)
	{
		float max_sep = 0.0f;
		for (size_t i = 0; i < num_active_elements; ++i)
		{
			float sep = separation(axis, active_elements[i], static_element, mtvs[i]);
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
				float sep = separation(axis, active_elements[i], static_elements[j], mtvs[i * num_static_elements + j]);
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

	static ContactResult param_contact_result(const Element& e1, const Element& e2, const CollisionResult& collision)
	{
		ContactResult contact{ .overlap = collision.overlap };
		if (contact.overlap)
		{
			contact.active_contact.impulse = collision.mtv();
			contact.passive_contact.impulse = -contact.active_contact.impulse;

			ContactManifold c1 = e1.deepest_manifold(-collision.unit_impulse);
			ContactManifold c2 = e2.deepest_manifold(collision.unit_impulse);
			glm::vec2 p1 = {}, p2 = {};
			ContactManifold::clamp(collision.unit_impulse, c1, c2, p1, p2);

			contact.active_contact.position = p1;
			contact.passive_contact.position = p2;
		}
		return contact;
	}

	ContactResult compound_contact(const Element* active_elements, const size_t num_active_elements,
		const Element& static_element, const CompoundPerfParameters perf)
	{
		const FixedVector<MTVPosNeg> mtvs = generate_mtvs(active_elements, num_active_elements, static_element);
		size_t most_significant_active_element = 0;
		CollisionResult collision = compound_collision_generic([&](UnitVector2D axis)
			{ return separation(axis, active_elements, num_active_elements, static_element, most_significant_active_element, mtvs); }, perf);
		return param_contact_result(active_elements[most_significant_active_element], static_element, collision);
	}

	ContactResult compound_contact(const Element* active_elements, const size_t num_active_elements,
		const Element* static_elements, const size_t num_static_elements, const CompoundPerfParameters perf)
	{
		const FixedVector<MTVPosNeg> mtvs = generate_mtvs(active_elements, num_active_elements, static_elements, num_static_elements);
		size_t most_significant_active_element = 0, most_significant_static_element = 0;
		CollisionResult collision = compound_collision_generic([&](UnitVector2D axis)
			{ return separation(axis, active_elements, num_active_elements, static_elements, num_static_elements,
				most_significant_active_element, most_significant_static_element, mtvs); }, perf);
		return param_contact_result(active_elements[most_significant_active_element], static_elements[most_significant_static_element], collision);
	}
}
