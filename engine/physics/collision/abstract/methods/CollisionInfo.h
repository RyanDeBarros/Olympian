#pragma once

#include "external/GLM.h"

#include <vector>
#include <variant>

namespace oly::acm2d
{
	struct OverlapResult
	{
		bool overlap;

		OverlapResult(bool overlap = bool()) : overlap(overlap) {}

		operator bool () const { return overlap; }
	};
	
	struct CollisionResult
	{
		bool overlap;
		float penetration_depth;
		glm::vec2 unit_impulse;

		// minimum translation vector
		glm::vec2 mtv() const { return unit_impulse * penetration_depth; }
		CollisionResult& invert() { unit_impulse *= -1; return *this; }
	};

	struct ContactResult
	{
		struct Feature
		{
			glm::vec2 position;
			glm::vec2 impulse;
		};

		bool overlap;
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

	private:
		glm::vec2 _direction;

	public:
		glm::vec2 direction() const { return _direction; }
		void set_direction(glm::vec2 direction) { _direction = glm::normalize(direction); }

		float clip = 0.0f;
	};

	struct RaycastResult
	{
		bool hit;
		glm::vec2 contact;
	};
}
