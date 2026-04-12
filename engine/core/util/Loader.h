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

	extern Polymorphic<TransformModifier2D> load_transform_modifier_2d(TOMLNode node);

	extern bool parse_color(const StringParam& text, glm::vec4& color);
}
