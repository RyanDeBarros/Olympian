#pragma once

namespace oly
{
	constexpr unsigned int unsigned_mod(int pos, int mod)
	{
		pos = pos % mod;
		return pos >= 0 ? pos : pos + mod;
	}

	constexpr int roundi(float v)
	{
		return (v > 0.0f) ? static_cast<int>(v + 0.5f) : static_cast<int>(v - 0.5f);
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
}

template<>
struct std::hash<oly::BigSize>
{
	size_t operator()(const oly::BigSize& bgz) const { return bgz.hash(); }
};
