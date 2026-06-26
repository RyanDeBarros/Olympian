#pragma once

#include <optional>
#include <ostream>

namespace oly::editor
{
	template<typename T>
	struct OptionalPrimitive
	{
		bool has_value;
		T value;

		constexpr std::optional<T> Opt() const
		{
			if (has_value)
				return value;
			else
				return std::nullopt;
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
		return OptionalPrimitive<T>{.has_value = false };
	}

	template<typename T>
	constexpr OptionalPrimitive<T> MakeOpt(T value)
	{
		return OptionalPrimitive<T>{.has_value = true, .value = value };
	}

	using OptionalInt = OptionalPrimitive<int>;
	using OptionalFloat = OptionalPrimitive<float>;
	using OptionalDouble = OptionalPrimitive<double>;
}
