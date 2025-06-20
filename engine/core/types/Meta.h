#pragma once

#include <memory>
#include <variant>
#include <optional>

namespace oly
{
	template<typename T>
	constexpr T max(const T& first, const T& second)
	{
		return first >= second ? first : second;
	}

	template<typename T, typename... Rest>
	inline T max(const T& first, const Rest&... rest)
	{
		return max(first, max(rest...));
	}

	namespace internal
	{
		template<size_t I, typename T, typename... Rest>
		constexpr const T& get_by_index(const T& first, const Rest&... rest) {
			if constexpr (I == 0)
				return first;
			else
				return get_by_index<I - 1>(rest...);
		}
	}

	template<typename T>
	constexpr size_t max_of(const T&)
	{
		return 0;
	}

	template<typename T, typename... Rest>
	inline size_t max_of(const T& first, const Rest&... rest)
	{
		return first >= internal::get_by_index<0>(rest...) ? 0 : 1 + max_of(rest...);
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
}
