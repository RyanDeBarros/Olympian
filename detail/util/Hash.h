#pragma once

#include <array>

namespace oly
{
	struct Hasher
	{
		size_t h = 0;

		static void hash_combine(size_t& hash, size_t with);

		template<typename T>
		Hasher& with(const T& with)
		{
			hash_combine(h, std::hash<T>{}(with));
			return *this;
		}

		operator size_t () const
		{
			return h;
		}
	};

	template<typename T>
	struct ArrayHash
	{
		template<size_t N>
		size_t operator()(const std::array<T, N>& a) const
		{
			Hasher h;
			for (size_t i = 0; i < N; ++i)
				h.with(a[i]);
			return h;
		}
	};
}
