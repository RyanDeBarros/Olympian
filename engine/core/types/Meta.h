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

	template<typename ToVariant, typename FromVariant>
	inline ToVariant convert_variant(const FromVariant& from)
	{
		return std::visit([](const auto& f) { return ToVariant{ f }; }, from);
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

	template<typename T, typename Class>
	constexpr bool visiting_class_is = std::is_same_v<std::decay_t<T>, Class>;
}
