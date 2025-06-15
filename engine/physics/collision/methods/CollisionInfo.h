#pragma once

#include "core/base/UnitVector.h"
#include "physics/collision/Tolerance.h"

#include <vector>

namespace oly::col2d
{
	struct OverlapResult
	{
		bool overlap = false;

		OverlapResult(bool overlap = bool()) : overlap(overlap) {}

		operator bool () const { return overlap; }
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
		Feature active_feature, static_feature;

		ContactResult& invert()
		{
			active_feature.impulse *= -1.0f;
			static_feature.impulse *= -1.0f;
			std::swap(active_feature, static_feature);
			return *this;
		}
	};

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

	extern CollisionResult greedy_collision(const std::vector<CollisionResult>& collisions);
	extern ContactResult greedy_contact(const std::vector<ContactResult>& contacts);
}
