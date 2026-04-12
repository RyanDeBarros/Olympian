#pragma once

#include "external/GL.h"
#include "external/TOML.h"
#include "core/base/Transforms.h"
#include "core/util/ResourcePath.h"
#include "core/util/StringParam.h"

#include <array>
#include <cstdint>
#include <string>
#include <algorithm>

namespace oly::io
{
	extern toml::v3::parse_result load_toml(const ResourcePath& file);

	template<typename T>
	std::optional<T> parse(TOMLNode node);

	template<typename T>
	bool try_parse(TOMLNode node, T& obj)
	{
		if (auto o = parse<T>(node))
		{
			obj = *o;
			return true;
		}
		else
			return false;
	}

	template<typename T>
	T parse_or(TOMLNode node, T def)
	{
		auto o = parse<T>(node);
		return o ? *o : def;
	}

	namespace internal
	{
		extern void log_context_warning(const StringParam& msg);
		extern void log_context_error(const StringParam& msg);
	}

	template<typename T>
	std::optional<T> parse_or_warn(TOMLNode node, const StringParam& warning)
	{
		auto o = parse<T>(node);
		if (!o)
			internal::log_context_warning(warning);
		return o;
	}

	template<typename T>
	T parse_or_throw(TOMLNode node, const StringParam& error)
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
	std::optional<T> parse_if_exists(TOMLNode node, Enum key)
	{
		if (auto value = io::parse_key(node, key))
			return io::parse_or_warn<T>(value, "cannot parse " + io::key_string(key) + " field");
		else
			return std::nullopt;
	}

	template<typename T, typename Enum>
	bool parse_if_exists(TOMLNode node, Enum key, T& obj)
	{
		if (auto o = parse_if_exists<T, Enum>(node, key))
		{
			obj = *o;
			return true;
		}
		else
			return false;
	}

	template<typename T, typename Enum>
	T parse_required(TOMLNode node, Enum key, const StringParam& error_suffix = "")
	{
		// TODO v7 DeferredStringParam that computes string catenation only when it's required to be used (holds a Variant<StringParam, std::vector<StringParam>>) overload << for clarity in appending strings -> or variadic constructor arguments?
		std::stringstream ss;
		ss << "cannot parse " << io::key_string(key) << " field" << (error_suffix.empty() ? "" : " ") << error_suffix;
		return io::parse_or_throw<T>(io::parse_key(node, key), ss.str());
	}

	template<typename T, typename Enum>
	T parse_optional(TOMLNode node, Enum key, T def, const StringParam& warning_suffix = "")
	{
		if (auto value = io::parse_key(node, key))
		{
			std::stringstream ss;
			ss << "cannot parse " << io::key_string(key) << " field" << (warning_suffix.empty() ? "" : " ") << warning_suffix;
			if (auto obj = io::parse_or_warn<T>(value, ss.str()))
				return *obj;
		}

		return def;
	}

	template<typename Enum>
	TOMLNode parse_required_node(TOMLNode node, Enum key, const StringParam& error_suffix = "")
	{
		if (auto o = io::parse_key(node, key))
			return o;

		std::stringstream ss;
		ss << "missing " << io::key_string(key) << " table" << (error_suffix.empty() ? "" : " ") << error_suffix;
		internal::log_context_error(ss.str());
		throw Error(ErrorCode::LoadAsset);
	}

	// TODO v7 move to Transform
	extern Polymorphic<TransformModifier2D> load_transform_modifier_2d(TOMLNode node);

	// TODO v7 move to different file
	extern bool parse_color(const StringParam& text, glm::vec4& color);
}
