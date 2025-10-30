#pragma once

#include <random>

#include "core/types/Meta.h"

namespace oly
{
	template<numeric T>
	struct Random
	{
	private:
		using RNGEngineType = std::conditional_t<(sizeof(T) > 4), std::mt19937_64, std::mt19937>;

		static RNGEngineType engine;

	public:
		static T range(T min, T max) requires std::integral<T>
		{
			std::uniform_int_distribution<T> dist(min, max);
			return dist(engine);
		}

		static T range(T min, T max) requires std::floating_point<T>
		{
			std::uniform_real_distribution<T> dist(min, max);
			return dist(engine);
		}
	};

	template<numeric T>
	typename Random<T>::RNGEngineType Random<T>::engine{ std::random_device{}() };
}
