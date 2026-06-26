#pragma once

#include <array>
#include <ostream>

#include <glm/glm.hpp>

namespace oly::editor
{
	template<typename T, size_t N>
	std::ostream& operator<<(std::ostream& os, const std::array<T, N>& array)
	{
		os << "Array[ ";

		for (size_t i = 0; i < N; ++i)
		{
			os << array[i];

			if (i + 1 < N)
				os << ", ";
		}

		return os << " ]";
	}

	template<glm::length_t L>
	std::ostream& operator<<(std::ostream& os, glm::vec<L, float> vec)
	{
		os << "Vec" << L << "(";

		for (glm::length_t i = 0; i < L; ++i)
		{
			os << vec[i];

			if (i + 1 < L)
				os << ", ";
		}

		return os << ")";
	}
}
