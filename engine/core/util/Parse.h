#pragma once

#include "external/TOML.h"
#include "external/GL.h"
#include "core/util/DeferredStringParam.h"

#include <array>
#include <cstdint>
#include <string>
#include <algorithm>

namespace oly::io
{
	template<typename T>
	struct PartialView
	{
		T& val;

		const T& operator*() const { return val; }
		T& operator*() { return val; }
		const T* operator->() const { return &val; }
		T* operator->() { return &val; }
	};

	template<typename T>
	bool try_parse(TOMLNode node, T& obj);

	template<typename T>
	bool try_parse(TOMLNode node, PartialView<T> obj);

	template<typename T>
	std::optional<T> parse(TOMLNode node)
	{
		T obj;
		if (try_parse(node, obj))
			return obj;
		else
			return std::nullopt;
	}

	template<typename T>
	T parse_or(TOMLNode node, T def)
	{
		try_parse(node, def);
		return def;
	}

	namespace internal
	{
		extern void log_context_warning(const DeferredStringParam& msg);
		extern void log_context_error(const DeferredStringParam& msg);
	}

	template<typename T>
	bool try_parse_or_warn(TOMLNode node, T& obj, const DeferredStringParam& warning)
	{
		if (try_parse(node, obj))
			return true;

		internal::log_context_warning(warning);
		return false;
	}

	template<typename T>
	bool try_parse_or_warn(TOMLNode node, PartialView<T> obj, const DeferredStringParam& warning)
	{
		if (try_parse(node, obj))
			return true;

		internal::log_context_warning(warning);
		return false;
	}

	template<typename T>
	std::optional<T> parse_or_warn(TOMLNode node, const DeferredStringParam& warning)
	{
		T obj;
		if (try_parse_or_warn(node, obj, warning))
			return obj;
		else
			return std::nullopt;
	}

	template<typename T>
	T parse_or_throw(TOMLNode node, const DeferredStringParam& error)
	{
		auto o = parse<T>(node);
		if (o)
			return *o;

		internal::log_context_error(error);
		throw Error(ErrorCode::LoadAsset);
	}

	template<typename Enum>
	constexpr std::string key_string(Enum key)
	{
		const auto value = static_cast<std::underlying_type_t<Enum>>(key);
		constexpr size_t KeySize = OLYMPIAN_ENGINE_ASSET_KEY_SIZE;
		std::array<char, KeySize> bytes;

		for (size_t i = 0; i < KeySize; ++i)
			bytes[KeySize - 1 - i] = char((value >> (i * KeySize)) & 0xFF);

		return std::string(bytes.begin(), std::find(bytes.begin(), bytes.end(), '\0'));
	}

	template<typename Enum>
	constexpr TOMLNode parse_key(TOMLNode node, Enum key)
	{
		return node[key_string(key)];
	}

	template<typename T, typename Enum>
	bool try_parse_if_exists(TOMLNode node, Enum key, T& obj, const DeferredStringParam& warning_suffix = {})
	{
		if (auto value = parse_key(node, key))
		{
			DeferredStringParam warning{ "cannot parse ", io::key_string(key), " field", warning_suffix.empty() ? "" : " " };
			return try_parse_or_warn(value, obj, warning << warning_suffix);
		}
		else
			return false;
	}

	template<typename T, typename Enum>
	bool try_parse_if_exists(TOMLNode node, Enum key, PartialView<T> obj, const DeferredStringParam& warning_suffix = {})
	{
		if (auto value = parse_key(node, key))
		{
			DeferredStringParam warning{ "cannot parse ", io::key_string(key), " field", warning_suffix.empty() ? "" : " " };
			return try_parse_or_warn(value, obj, warning << warning_suffix);
		}
		else
			return false;
	}

	template<typename T, typename Enum>
	std::optional<T> parse_if_exists(TOMLNode node, Enum key, const DeferredStringParam& warning_suffix = {})
	{
		T obj;
		if (try_parse_if_exists(node, key, obj, warning_suffix))
			return obj;
		else
			return std::nullopt;
	}

	template<typename T, typename Enum>
	T parse_if_exists_or(TOMLNode node, Enum key, T def, const DeferredStringParam& warning_suffix = {})
	{
		try_parse_if_exists(node, key, def, warning_suffix);
		return def;
	}

	template<typename T, typename Enum>
	T parse_required(TOMLNode node, Enum key, const DeferredStringParam& error_suffix = {})
	{
		DeferredStringParam error{ "cannot parse ", io::key_string(key), " field", error_suffix.empty() ? "" : " " };
		return io::parse_or_throw<T>(io::parse_key(node, key), error << error_suffix);
	}

	template<typename T, typename Enum>
	T parse_optional(TOMLNode node, Enum key, T def, const DeferredStringParam& warning_suffix = {})
	{
		if (auto value = io::parse_key(node, key))
		{
			DeferredStringParam warning{ "cannot parse ", io::key_string(key), " field", warning_suffix.empty() ? "" : " " };
			if (auto obj = io::parse_or_warn<T>(value, warning << warning_suffix))
				return *obj;
		}

		return def;
	}

	template<typename Enum>
	TOMLNode parse_required_node(TOMLNode node, Enum key, const DeferredStringParam& error_suffix = {})
	{
		if (auto o = io::parse_key(node, key))
			return o;

		DeferredStringParam error{ "missing ", io::key_string(key), " table", error_suffix.empty() ? "" : " " };
		internal::log_context_error(error << error_suffix);
		throw Error(ErrorCode::LoadAsset);
	}

	template<typename Translator, typename Enum>
	bool try_parse_enum(TOMLNode node, Enum key, typename Translator::EnumType& obj, const DeferredStringParam& warning_suffix = {})
	{
		if (auto value = parse<typename Translator::IndexType>(parse_key(node, key)))
		{
			try
			{
				obj = Translator::val(*value);
				return true;
			}
			catch (const std::out_of_range&)
			{
				DeferredStringParam warning{ "unrecognized ", io::key_string(key), " enum (", *value, ")", warning_suffix.empty() ? "" : " " };
				internal::log_context_warning(warning << warning_suffix); // TODO v7 should be error -> return false, otherwise use default below instead of returning false
			}
		}

		return false;
	}

	template<typename Translator, typename Enum>
	std::optional<typename Translator::EnumType> parse_enum(TOMLNode node, Enum key, const DeferredStringParam& warning_suffix = {})
	{
		typename Translator::EnumType obj;
		if (try_parse_enum<Translator, Enum>(node, key, obj, warning_suffix))
			return obj;
		else
			return std::nullopt;
	}

	template<typename Translator, typename Enum>
	typename Translator::EnumType parse_required_enum(TOMLNode node, Enum key, const DeferredStringParam& error_suffix = {})
	{
		auto value = parse_required<typename Translator::IndexType>(node, key, error_suffix);
		try
		{
			return Translator::val(value);
		}
		catch (const std::out_of_range&)
		{
			DeferredStringParam error{ "unrecognized ", io::key_string(key), " enum (", value, ")", error_suffix.empty() ? "" : " " };
			internal::log_context_error(error << error_suffix);
			throw Error(ErrorCode::LoadAsset);
		}
	}

	template<typename Translator, typename Enum>
	typename Translator::EnumType parse_optional_enum(TOMLNode node, Enum key, typename Translator::EnumType def, const DeferredStringParam& warning_suffix = {})
	{
		try_parse_enum<Translator, Enum>(node, key, def, warning_suffix);
		return def;
	}
}
