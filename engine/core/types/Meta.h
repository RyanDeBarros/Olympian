#pragma once

#include <memory>
#include <variant>
#include <optional>

namespace oly
{
	template<typename T>
	constexpr T max(const T& first, const T& second)
	{
		return first > second ? first : second;
	}

	template<typename T, typename... Rest>
	inline T max(const T& first, const Rest&... rest)
	{
		return max(first, max(rest...));
	}

	template<typename T>
	constexpr T min(const T& first, const T& second)
	{
		return first < second ? first : second;
	}

	template<typename T, typename... Rest>
	inline T min(const T& first, const Rest&... rest)
	{
		return min(first, min(rest...));
	}

	namespace internal
	{
		template<typename Tuple, size_t... Is>
		constexpr size_t max_of_impl(const Tuple& tup, std::index_sequence<Is...>)
		{
			size_t max_index = 0;
			auto max_value = std::get<0>(tup);
			((std::get<Is>(tup) > max_value ? (max_value = std::get<Is>(tup), max_index = Is, void(0)) : void(0)), ...);
			return max_index;
		}

		template<typename Tuple, size_t... Is>
		constexpr size_t min_of_impl(const Tuple& tup, std::index_sequence<Is...>)
		{
			size_t min_index = 0;
			auto min_value = std::get<0>(tup);
			((std::get<Is>(tup) < min_value ? (min_value = std::get<Is>(tup), min_index = Is, void(0)) : void(0)), ...);
			return min_index;
		}
	}

	template<typename... Args>
	constexpr size_t max_of(const Args&... args)
	{
		static_assert(sizeof...(Args) > 0, "At least one argument required for oly::max_of");
		return internal::max_of_impl(std::tie(args...), std::make_index_sequence<sizeof...(Args)>{});
	}

	template<typename... Args>
	constexpr size_t min_of(const Args&... args)
	{
		static_assert(sizeof...(Args) > 0, "At least one argument required for oly::min_of");
		return internal::min_of_impl(std::tie(args...), std::make_index_sequence<sizeof...(Args)>{});
	}

	template<typename T>
	inline T dupl(const T& obj)
	{
		return obj;
	}

	template<typename T>
	inline T dupl(T&& obj)
	{
		return std::move(obj);
	}

	template<typename Struct, typename Member>
	constexpr size_t member_offset(Member Struct::* member)
	{
		return reinterpret_cast<std::size_t>(&(reinterpret_cast<Struct*>(0)->*member));
	}

	template<typename T>
	concept numeric = std::integral<T> || std::floating_point<T>;

	template<typename T>
	inline std::shared_ptr<T> move_shared(T&& obj) { return std::make_shared<T>(std::move(obj)); }
	template<typename T>
	inline std::unique_ptr<T> move_unique(T&& obj) { return std::make_unique<T>(std::move(obj)); }

	template<typename ToOptional, typename FromOptional>
	inline std::optional<ToOptional> convert_optional(const std::optional<FromOptional>& from)
	{
		return from ? std::make_optional<ToOptional>((ToOptional)*from) : std::nullopt;
	}

	template<typename T, typename... Class>
	constexpr bool visiting_class_is = std::disjunction_v<std::is_same<std::decay_t<T>, Class>...>;

	template<typename From, typename To>
	concept PointerConvertibleTo = std::convertible_to<From*, To*>;
}
