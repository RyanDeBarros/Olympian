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

		static void clamp(UnitVector2D axis, const ContactManifold c1, const ContactManifold c2, glm::vec2& p1, glm::vec2& p2)
		{
			UnitVector2D tangent = axis.get_quarter_turn();

			float v11 = tangent.dot(c1.p1);
			float v12 = c1.single ? v11 : tangent.dot(c1.p2);
			if (v11 > v12)
				std::swap(v11, v12);
			float v21 = tangent.dot(c2.p1);
			float v22 = c2.single ? v21 : tangent.dot(c2.p2);
			if (v21 > v22)
				std::swap(v21, v22);

			float w1, w2;
			if (v12 < v21)
			{
				w1 = v12;
				w2 = v21;
			}
			else if (v11 > v22)
			{
				w1 = v11;
				w2 = v22;
			}
			else
			{
				v11 = glm::clamp(v11, v21, v22);
				v12 = glm::clamp(v12, v21, v22);
				w1 = 0.5f * (v11 + v12);
				w2 = w1;
			}

			glm::vec2 along_axis = axis.dot(c1.p2) * (glm::vec2)axis;
			p1 = along_axis + w1 * (glm::vec2)tangent;
			p2 = along_axis + w2 * (glm::vec2)tangent;
		}
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
		struct Contact
		{
			glm::vec2 position = { 0.0f, 0.0f };
			glm::vec2 impulse = { 0.0f, 0.0f };
		};

		bool overlap = false;
		Contact active_contact, passive_contact;

		ContactResult& invert() { std::swap(active_contact, passive_contact); return *this; }
	};

	template<typename C1, typename C2>
	inline ContactResult standard_contact_result(const C1& c1, const C2& c2, UnitVector2D axis, float depth)
	{
		ContactResult contact{ .overlap = true };
		contact.active_contact.impulse = depth * (glm::vec2)axis;
		contact.passive_contact.impulse = -contact.active_contact.impulse;

		ContactManifold m1 = c1.deepest_manifold(-axis);
		ContactManifold m2 = c2.deepest_manifold(axis);
		glm::vec2 p1 = {}, p2 = {};
		ContactManifold::clamp(axis, m1, m2, p1, p2);

		contact.active_contact.position = p1;
		contact.passive_contact.position = p2;
		return contact;
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
