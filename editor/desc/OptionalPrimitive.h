#pragma once

#include <optional>

namespace oly::editor
{
	template<typename T>
	struct OptionalPrimitive
	{
		const bool has_value;
		const T value;

		constexpr std::optional<T> Opt() const
		{
			if (has_value)
				return value;
			else
				return std::nullopt;
		}
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
}
