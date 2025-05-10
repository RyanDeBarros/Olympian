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
}
