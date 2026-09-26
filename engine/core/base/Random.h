#pragma once

#include <imp/traits.hpp>

#include <random>

namespace oly
{
	template<imp::numeric T>
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

        static T range01() requires std::floating_point<T>
        {
            return range(T(0), T(1));
        }

        static T angle() requires std::floating_point<T>()
        {
            return range(0, 2 * glm::pi<T>());
        }

        static glm::vec<2, T> unit_circle() requires std::floating_point<T>
        {
            T rad = angle();
            return glm::vec<2, T>{ glm::cos(rad), glm::sin(rad) };
        }

        static glm::vec<2, T> unit_disk() requires std::floating_point<T>
        {
            return range01() * unit_circle();
        }

        static glm::vec<3, T> unit_sphere() requires std::floating_point<T>
        {
            T theta = angle();
            T phi = 0.5f * angle();
            return glm::vec<3, T>{ glm::sin(phi) * glm::cos(theta), glm::sin(phi) * glm::sin(theta), glm::cos(phi) };
        }

        static glm::vec<3, T> unit_ball() requires std::floating_point<T>
        {
            return range01() * unit_sphere();
        }
	};

	template<imp::numeric T>
	typename Random<T>::RNGEngineType Random<T>::engine{ std::random_device{}() };
}
