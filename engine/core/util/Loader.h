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

	// TODO v7 make these templated?

	extern bool parse_bool(TOMLNode node, bool& v);
	extern void parse_bool(TOMLNode node, std::optional<bool>& v);
	inline std::optional<bool> parse_bool(TOMLNode node) { bool v; if (parse_bool(node, v)) return v; else return std::nullopt; }
	inline bool parse_bool_or(TOMLNode node, bool def) { parse_bool(node, def); return def; }

	extern bool parse_int(TOMLNode node, int& v);
	extern void parse_int(TOMLNode node, std::optional<int>& v);
	inline std::optional<int> parse_int(TOMLNode node) { int v; if (parse_int(node, v)) return v; else return std::nullopt; }
	inline int parse_int_or(TOMLNode node, int def) { parse_int(node, def); return def; }

	extern bool parse_uint(TOMLNode node, unsigned int& v);
	extern void parse_uint(TOMLNode node, std::optional<unsigned int>& v);
	inline std::optional<unsigned int> parse_uint(TOMLNode node) { unsigned int v; if (parse_uint(node, v)) return v; else return std::nullopt; }
	inline unsigned int parse_uint_or(TOMLNode node, unsigned int def) { parse_uint(node, def); return def; }

	extern bool parse_float(TOMLNode node, float& v);
	extern void parse_float(TOMLNode node, std::optional<float>& v);
	inline std::optional<float> parse_float(TOMLNode node) { float v; if (parse_float(node, v)) return v; else return std::nullopt; }
	inline float parse_float_or(TOMLNode node, float def) { parse_float(node, def); return def; }

	extern bool parse_double(TOMLNode node, double& v);
	extern void parse_double(TOMLNode node, std::optional<double>& v);
	inline std::optional<double> parse_double(TOMLNode node) { double v; if (parse_double(node, v)) return v; else return std::nullopt; }
	inline double parse_double_or(TOMLNode node, double def) { parse_double(node, def); return def; }

	extern bool parse_size_t(TOMLNode node, size_t& v);
	extern void parse_size_t(TOMLNode node, std::optional<size_t>& v);
	inline std::optional<size_t> parse_size_t(TOMLNode node) { size_t v; if (parse_size_t(node, v)) return v; else return std::nullopt; }
	inline size_t parse_size_t_or(TOMLNode node, size_t def) { parse_size_t(node, def); return def; }

	template<size_t N>
	inline bool parse_vec(TOMLNode node, glm::vec<N, float>& v)
	{
		auto arr = node.as_array();
		if (arr && arr->size() == N)
		{
			glm::vec<N, float> u;
			for (int i = 0; i < N; ++i)
			{
				if (auto d = arr->get_as<double>(i))
					u[i] = (float)d->get();
				else if (auto n = arr->get_as<int64_t>(i))
					u[i] = (float)n->get();
				else
					return false;
			}
			v = u;
			return true;
		}
		return false;
	}

	template<size_t N>
	inline bool parse_ivec(TOMLNode node, glm::vec<N, int>& v)
	{
		auto arr = node.as_array();
		if (arr && arr->size() == N)
		{
			glm::vec<N, int> u;
			for (int i = 0; i < N; ++i)
			{
				if (auto n = arr->get_as<int64_t>(i))
					u[i] = (int)n->get();
				else
					return false;
			}
			v = u;
			return true;
		}
		return false;
	}

	template<typename Enum>
	constexpr auto parse_key(TOMLNode node, Enum key)
	{
		const auto value = static_cast<std::underlying_type_t<Enum>>(key);
		constexpr size_t KeySize = OLYMPIAN_ENGINE_ASSET_KEY_SIZE;
		std::array<char, KeySize> bytes;

		for (size_t i = 0; i < KeySize; ++i)
			bytes[KeySize - 1 - i] = char((value >> (i * KeySize)) & 0xFF);

		return node[std::string(bytes.begin(), std::find(bytes.begin(), bytes.end(), '\0'))];
	}

	extern Polymorphic<TransformModifier2D> load_transform_modifier_2d(TOMLNode node);

	extern bool parse_color(const StringParam& text, glm::vec4& color);
}
