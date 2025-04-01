#pragma once

namespace oly
{
	constexpr unsigned int unsigned_mod(int pos, int mod)
	{
		pos = pos % mod;
		return pos >= 0 ? pos : pos + mod;
	}

	template<typename T>
	inline T dupl(const T& obj)
	{
		return obj;
	}

	template<typename Struct, typename Member>
	constexpr std::size_t member_offset(Member Struct::* member)
	{
		return reinterpret_cast<std::size_t>(&(reinterpret_cast<Struct*>(0)->*member));
	}

	template<typename T>
	struct Range
	{
		T initial = T();
		T diff = T();

		T last() const { return initial + diff; }
	};
}
