#pragma once

#include "external/GLM.h"

#include <imp/traits.hpp>

namespace oly
{
	template<typename T>
	struct Distance
	{
		static_assert(requires { false; }, "Distance not implemented for this type");
	};

	template<imp::numeric T>
	struct Distance<T>
	{
		double operator()(const T& a, const T& b) const { return static_cast<double>(std::abs(a - b)); }
	};

	template<glm::length_t L, typename T, enum glm::qualifier Q>
	struct Distance<glm::vec<L, T, Q>>
	{
		double operator()(const glm::vec<L, T, Q>& a, const glm::vec<L, T, Q>& b) const { return static_cast<double>(glm::length(a - b)); }
	};

	template<typename T>
	struct ToleranceImpl
	{
		static_assert(requires { false; }, "Tolerance not implemented for this type");
	};

	template<typename T>
	constexpr double Tolerance = ToleranceImpl<T>::TOL;

	template<imp::numeric T>
	struct ToleranceImpl<T>
	{
		static constexpr double TOL = 1e-7;
	};

	template<>
	struct ToleranceImpl<double>
	{
		static constexpr double TOL = 1e-15;
	};

	template<glm::length_t L, typename T, enum glm::qualifier Q>
	struct ToleranceImpl<glm::vec<L, T, Q>>
	{
		static constexpr double TOL = 1e-7;
	};

	template<glm::length_t L, enum glm::qualifier Q>
	struct ToleranceImpl<glm::vec<L, double, Q>>
	{
		static constexpr double TOL = 1e-15;
	};

	template<typename T>
	inline bool approx(const T& a, const T& b, double tolerance = Tolerance<T>)
	{
		return Distance<T>{}(a, b) <= tolerance;
	}

	template<typename T>
	inline bool near_zero(const T& a, double tolerance = Tolerance<T>)
	{
		return approx(a, T(0), tolerance);
	}

	template<typename T>
	inline bool above_zero(const T& a, double tolerance = Tolerance<T>)
	{
		return static_cast<double>(a) > tolerance;
	}

	template<typename T>
	inline bool below_zero(const T& a, double tolerance = Tolerance<T>)
	{
		return static_cast<double>(a) < tolerance;
	}

	template<imp::numeric T>
	inline bool near_multiple(const T& a, const T& b, double tolerance = Tolerance<T>)
	{
		if (near_zero(b))
			return false;

		double m = fmod(fmod(double(a), double(b)) + double(b), double(b));
		return approx(m, 0.0, tolerance) || approx(m, double(b), tolerance);
	}
}
