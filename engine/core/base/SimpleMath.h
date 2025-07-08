#pragma once

#include <utility>

#include "core/base/Constants.h"

namespace oly
{
	constexpr unsigned int unsigned_mod(int pos, int mod)
	{
		pos = pos % mod;
		return pos >= 0 ? pos : pos + mod;
	}

	constexpr unsigned int unsigned_mod(int pos, size_t mod)
	{
		return unsigned_mod(pos, (int)mod);
	}

	constexpr int roundi(float v)
	{
		return (v >= 0.0f) ? static_cast<int>(v + 0.5f) : static_cast<int>(v - 0.5f);
	}

	class BigSize
	{
		size_t order = 0;
		size_t v;

	public:
		BigSize(size_t v = 0) : v(v) {}
		
		BigSize& operator++() { if (v == SIZE_MAX) { ++order; v = 0; } else { ++v; } return *this; }
		BigSize operator++(int) { BigSize s = *this; ++*this; return s; }
		bool operator==(const BigSize&) const = default;
		bool operator<(const BigSize& rhs) const { return order < rhs.order || (order == rhs.order && v < rhs.order); }

		size_t get_order() const { return order; }
		size_t get_remainder() const { return v; }
		size_t hash() const { return std::hash<size_t>{}(v) ^ (std::hash<size_t>{}(order) << 1); }
	};

	typedef std::pair<float, float> fpair;

	template<typename T, T Min, T Max>
	struct BoundedValue
	{
	private:
		T val;

	public:
		BoundedValue(T val = Min) { set(val); }

		T get() const { return val; }
		void set(T v) { val = glm::clamp(v, Min, Max); }
		operator T() const { return val; }
		BoundedValue<T, Min, Max>& operator=(T v) { set(v); return *this; }
	};

	template<float Min, float Max>
	using BoundedFloat = BoundedValue<float, Min, Max>;
	template<int Min, int Max>
	using BoundedInt = BoundedValue<int, Min, Max>;
	template<unsigned int Min, unsigned int Max>
	using BoundedUInt = BoundedValue<unsigned int, Min, Max>;

	using PositiveFloat = BoundedFloat<0.0f, nmax<float>()>;
}

template<>
struct std::hash<oly::BigSize>
{
	size_t operator()(const oly::BigSize& bgz) const { return bgz.hash(); }
};
