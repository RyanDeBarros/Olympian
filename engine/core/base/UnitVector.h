#pragma once

#include "external/GLM.h"

namespace oly
{
	class UnitVector2D
	{
		glm::vec2 _direction;

		struct _right{};
		struct _up{};
		struct _left{};
		struct _down{};
		struct _direct{};

	public:
		UnitVector2D() : _direction({ 1.0f, 0.0f }) {}
		UnitVector2D(glm::vec2 direction) : _direction(glm::normalize(direction)) {}
		UnitVector2D(float angle) : _direction(glm::cos(angle), glm::sin(angle)) {}

	private:
		UnitVector2D(glm::vec2 direction, _direct) : _direction(direction) {}
		UnitVector2D(_right) : _direction({ 1.0f, 0.0f }) {}
		UnitVector2D(_up) : _direction({ 0.0f, 1.0f }) {}
		UnitVector2D(_left) : _direction({ -1.0f, 0.0f }) {}
		UnitVector2D(_down) : _direction({ 0.0f, -1.0f }) {}

	public:
		static const UnitVector2D RIGHT;
		static const UnitVector2D UP;
		static const UnitVector2D LEFT;
		static const UnitVector2D DOWN;

		operator glm::vec2 () const { return _direction; }
		UnitVector2D& operator=(glm::vec2 direction) { _direction = glm::normalize(direction); return *this; }

		UnitVector2D operator-() const { return UnitVector2D(-_direction, _direct{}); }

		UnitVector2D& rotate(float angle) { float c = glm::cos(angle); float s = glm::sin(angle); _direction = glm::mat2{ { c, s }, { -s, c } } * _direction; return *this; }
		UnitVector2D get_rotated(float angle) const { float c = glm::cos(angle); float s = glm::sin(angle); return UnitVector2D(glm::mat2{ { c, s }, { -s, c } } * _direction, _direct{}); }
		UnitVector2D& quarter_turn() { _direction = { -_direction.y, _direction.x }; return *this; }
		UnitVector2D get_quarter_turn() const { UnitVector2D cpy = *this; return cpy.quarter_turn(); }

		float x() const { return _direction.x; }
		float y() const { return _direction.y; }
		float dot(glm::vec2 v) const { return glm::dot(v, _direction); }
	};

	inline const UnitVector2D UnitVector2D::RIGHT = UnitVector2D(_right{});
	inline const UnitVector2D UnitVector2D::UP = UnitVector2D(_up{});
	inline const UnitVector2D UnitVector2D::LEFT = UnitVector2D(_left{});
	inline const UnitVector2D UnitVector2D::DOWN = UnitVector2D(_down{});
}
