#pragma once

#include <optional>

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
