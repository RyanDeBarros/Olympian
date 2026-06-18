#pragma once

#include <utility>

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
}
