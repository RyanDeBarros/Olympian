#pragma once

#include "core/base/SimpleMath.h"
#include "core/types/Approximate.h"

namespace oly
{
	template<typename T, T Min, T Max>
	struct BoundedValue
	{
		static_assert(Min < Max, "BoundedValue minimum must be less than maximum.");

	private:
		T val;

	public:
		BoundedValue(T val = Min) { set(val); }

		T get() const { return val; }
		void set(T v) { val = glm::clamp(v, Min, Max); }
		operator T() const { return val; }
		BoundedValue<T, Min, Max>& operator=(T v) { set(v); return *this; }
	};

	template<typename T, T Min, T Max>
	struct Distance<BoundedValue<T, Min, Max>>
	{
		double operator()(const BoundedValue<T, Min, Max>& a, const BoundedValue<T, Min, Max>& b) const { return static_cast<double>(glm::abs(a - b)); }
	};

	template<typename T, T Min, T Max>
	struct Tolerance_V<BoundedValue<T, Min, Max>>
	{
		static constexpr double TOL = 1e-7;
	};

	template<float Min, float Max>
	using BoundedFloat = BoundedValue<float, Min, Max>;
	template<int Min, int Max>
	using BoundedInt = BoundedValue<int, Min, Max>;
	template<unsigned int Min, unsigned int Max>
	using BoundedUInt = BoundedValue<unsigned int, Min, Max>;

	using PositiveFloat = BoundedFloat<0.0f, nmax<float>()>;
	using StrictlyPositiveFloat = BoundedFloat<0.000001f, nmax<float>()>;

	struct PowerInterval
	{
	private:
		BoundedFloat<0.0f, 1.0f> _inner = 0.5f;
		BoundedFloat<0.0f, 1.0f> _outer = 0.5f;
		BoundedFloat<0.0f, 1.0f> _inner_midpoint = 0.5f;
		PositiveFloat _power = 1.0f;

		void sync_inner() { _inner = near_zero(_outer) ? 0.0f : near_zero(1.0f - _outer) ? 1.0f : glm::pow(_outer, _power); }

	public:
		PowerInterval(float inner_midpoint = 0.5f, float outer = 0.5f) : _outer(outer) { set_inner_midpoint(inner_midpoint); }

		float inner() const { return _inner; }
		float outer() const { return _outer; }
		void set_outer(float outer) { _outer = outer; sync_inner(); }

		PowerInterval& operator=(float outer) { set_outer(outer); return *this; }
		
		float inner_midpoint() const { return _inner_midpoint; }
		float power() const { return _power; }
		void set_inner_midpoint(float midpoint) { _inner_midpoint = midpoint; _power = glm::log(_inner_midpoint) / glm::log(0.5f); sync_inner(); }
		void set_power(float power) { _power = power; _inner_midpoint = glm::pow(0.5f, _power); sync_inner(); }
	};

	template<>
	struct Distance<PowerInterval>
	{
		double operator()(const PowerInterval& a, const PowerInterval& b) const { return static_cast<double>(glm::abs(a.inner() - b.inner())); }
	};

	template<>
	struct Tolerance_V<PowerInterval>
	{
		static constexpr double TOL = 1e-7;
	};
}
