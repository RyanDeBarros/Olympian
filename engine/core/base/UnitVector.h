#pragma once

#include "external/GLM.h"
#include "core/types/Approximate.h"

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
		bool operator==(const UnitVector2D& other) const { return _direction == other._direction; }
		bool operator<(const UnitVector2D&) const;
		bool operator<=(const UnitVector2D&) const;

		UnitVector2D& rotate(float angle) { float c = glm::cos(angle); float s = glm::sin(angle); _direction = glm::mat2{ { c, s }, { -s, c } } * _direction; return *this; }
		UnitVector2D get_rotated(float angle) const { float c = glm::cos(angle); float s = glm::sin(angle); return UnitVector2D(glm::mat2{ { c, s }, { -s, c } } * _direction, _direct{}); }
		UnitVector2D& quarter_turn() { _direction = { -_direction.y, _direction.x }; return *this; }
		UnitVector2D get_quarter_turn() const { UnitVector2D cpy = *this; return cpy.quarter_turn(); }
		UnitVector2D& flip_x() { _direction.x = -_direction.x; return *this; }
		UnitVector2D get_flipped_x() const { return UnitVector2D({ -_direction.x, _direction.y }, _direct{}); }
		UnitVector2D& flip_y() { _direction.y = -_direction.y; return *this; }
		UnitVector2D get_flipped_y() const { return UnitVector2D({ _direction.x, -_direction.y }, _direct{}); }

		float rotation() const { return glm::atan(_direction.y, _direction.x); }
		glm::mat2 rotation_matrix() const { return { _direction, { -_direction.y, _direction.x } }; }
		glm::mat2 inverse_rotation_matrix() const { return { { _direction.x, -_direction.y }, { _direction.y, _direction.x } }; }

		float x() const { return _direction.x; }
		float y() const { return _direction.y; }
		float dot(glm::vec2 v) const { return glm::dot(v, _direction); }
		bool near_standard(double tolerance = Tolerance<float>) const;

		UnitVector2D normal_project(glm::vec2 v) const;

		enum ParallelState
		{
			SAME_DIRECTION,
			OPPOSITE_DIRECTION,
			NON_PARALLEL
		};

		bool near_parallel(UnitVector2D other, double tolerance = Tolerance<float>) const;
		ParallelState near_parallel_state(UnitVector2D other, double tolerance = Tolerance<float>) const;

		size_t hash() const { return std::hash<glm::vec2>{}(_direction); }
	};

	inline const UnitVector2D UnitVector2D::RIGHT = UnitVector2D(_right{});
	inline const UnitVector2D UnitVector2D::UP = UnitVector2D(_up{});
	inline const UnitVector2D UnitVector2D::LEFT = UnitVector2D(_left{});
	inline const UnitVector2D UnitVector2D::DOWN = UnitVector2D(_down{});

	template<>
	struct Distance<UnitVector2D>
	{
		double operator()(const UnitVector2D& a, const UnitVector2D& b) const { return static_cast<double>(glm::abs(a.rotation() - b.rotation())); }
	};

	template<>
	struct Tolerance_V<UnitVector2D>
	{
		static constexpr double TOL = 1e-7;
	};
}

template<>
struct std::hash<oly::UnitVector2D>
{
	size_t operator()(const oly::UnitVector2D& v) const { return v.hash(); }
};
