#pragma once

#include <ostream>

namespace oly::editor
{
	template<typename T>
	struct OptionalPrimitive
	{
		bool has_value;
		T value;

		constexpr OptionalPrimitive(bool has_value = false, T value = T())
			: has_value(has_value), value(value)
		{
		}

		bool operator==(const OptionalPrimitive<T>&) const = default;
	};

	template<typename T>
	std::ostream& operator<<(std::ostream& os, const OptionalPrimitive<T>& opt)
	{
		os << "Optional[has_value=" << opt.has_value;
		
		if (opt.has_value)
			os << ", value=" << opt.value << "]";
		else
			os << "]";

		return os;
	}

	template<typename T>
	constexpr OptionalPrimitive<T> MakeOpt()
	{
		return OptionalPrimitive<T>(false, T());
	}

	template<typename T>
	constexpr OptionalPrimitive<T> MakeOpt(T value)
	{
		return OptionalPrimitive<T>(true, value);
	}

	using OptionalInt = OptionalPrimitive<int>;
	using OptionalFloat = OptionalPrimitive<float>;
	using OptionalDouble = OptionalPrimitive<double>;
}
