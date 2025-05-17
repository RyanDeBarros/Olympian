#pragma once

#include "external/GLM.h"

#include <vector>
#include <variant>

namespace oly::acm2d
{
	typedef bool OverlapInfo;
	
	struct GeometricInfo
	{
		OverlapInfo overlap;
		float penetration_depth;
		glm::vec2 unit_impulse;

		glm::vec2 mtv() const { return unit_impulse * penetration_depth; }
		GeometricInfo& inverted() { unit_impulse *= -1; return *this; }
	};

	struct StructuralInfo
	{
		enum
		{
			WITNESS = 0,
			CONTACT = 1,
			POINT = 0,
			LINE = 1
		};
		typedef std::pair<glm::vec2, glm::vec2> Line;
		typedef std::variant<glm::vec2, Line> ContactElement;
		typedef std::variant<glm::vec2, std::vector<ContactElement>> Manifold;

		GeometricInfo simple;
		Manifold active_manifold, static_manifold;

		bool is_contact() const { return simple.overlap; }
		bool is_witness() const { return !simple.overlap; }
		StructuralInfo& inverted() { simple.inverted(); return *this; }
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

	struct SimpleRayHit
	{
		bool hit;
		glm::vec2 contact;
	};

	struct DeepRayHit
	{
		bool hit;
		bool exit;
		float depth;
		glm::vec2 contact;
		glm::vec2 normal;
	};
}
