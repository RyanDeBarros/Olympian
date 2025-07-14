#pragma once

#include "core/base/UnitVector.h"
#include "physics/collision/Tolerance.h"

#include <vector>

namespace oly::col2d
{
	struct ContactManifold
	{
		glm::vec2 p1 = {}, p2 = {};
		bool single = true;
		glm::vec2 pt() const { return single ? p1 : 0.5f * (p1 + p2); }
	};

	struct OverlapResult
	{
		bool overlap = false;

		OverlapResult(bool overlap = bool()) : overlap(overlap) {}

		operator bool () const { return overlap; }

		bool operator!() const { return !overlap; }

		OverlapResult& invert() { return *this; }
	};

	struct CollisionResult
	{
		bool overlap = false;
		float penetration_depth = 0.0f;
		UnitVector2D unit_impulse;

		// minimum translation vector
		glm::vec2 mtv() const { return (glm::vec2)unit_impulse * penetration_depth; }
		CollisionResult& invert() { unit_impulse = -unit_impulse; return *this; }
	};

	struct ContactResult
	{
		struct Feature
		{
			glm::vec2 position = { 0.0f, 0.0f };
			glm::vec2 impulse = { 0.0f, 0.0f };
		};

		bool overlap = false;
		Feature active_feature, passive_feature;

		ContactResult& invert() { std::swap(active_feature, passive_feature); return *this; }
	};

	template<typename C1, typename C2>
	inline ContactResult standard_contact_result(const C1& c1, const C2& c2, UnitVector2D axis, float depth)
	{
		return {
			.overlap = true,
			.active_feature = {
				.position = c1.deepest_manifold(-axis).pt(),
				.impulse = depth * (glm::vec2)axis
			},
			.passive_feature = {
				.position = c2.deepest_manifold(axis).pt(),
				.impulse = -depth * (glm::vec2)axis
			}
		};
	}

	template<typename C1, typename C2>
	inline ContactResult standard_contact_result(const C1& c1, const C2& c2, const CollisionResult& collision)
	{
		if (collision.overlap)
			return standard_contact_result(c1, c2, collision.unit_impulse, collision.penetration_depth);
		else
			return { .overlap = false };
	}

	struct Ray
	{
		glm::vec2 origin;
		UnitVector2D direction;
		float clip = 0.0f;
	};

	struct RaycastResult
	{
		enum class Hit
		{
			NO_HIT,
			EMBEDDED_ORIGIN,
			TRUE_HIT
		} hit;
		glm::vec2 contact;
		UnitVector2D normal;
	};
}
