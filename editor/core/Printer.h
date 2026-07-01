#pragma once

#include <array>
#include <ostream>
#include <vector>

#include <glm/glm.hpp>

namespace oly
{
	template<typename T>
	struct StandardPrinter
	{
		void operator()(std::ostream& os, const T& obj) const
		{
			os << obj;
		}
	};

	template<typename T>
	struct StandardPrinter<std::vector<T>>
	{
		void operator()(std::ostream& os, const std::vector<T>& obj) const
		{
			os << "Vector[ ";

			for (auto it = obj.begin(); it != obj.end(); ++it)
			{
				os << *it;

				if (std::next(it) != obj.end())
					os << ", ";
			}

			os << " ]";
		}
	};

	template<typename T, size_t N>
	struct StandardPrinter<std::array<T, N>>
	{
		void operator()(std::ostream& os, const std::array<T, N>& obj)
		{
			os << "Array[ ";

			for (size_t i = 0; i < N; ++i)
			{
				os << obj[i];

				if (i + 1 < N)
					os << ", ";
			}

			os << " ]";
		}
	};

	template<glm::length_t L>
	struct StandardPrinter<glm::vec<L, float>>
	{
		void operator()(std::ostream& os, const glm::vec<L, float> obj)
		{
			os << "Vec" << L << "(";

			for (glm::length_t i = 0; i < L; ++i)
			{
				os << obj[i];

				if (i + 1 < L)
					os << ", ";
			}

			os << ")";
		}
	};
}
