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
}
