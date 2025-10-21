#pragma once

#include "external/GL.h"
#include "external/TOML.h"
#include "core/base/Transforms.h"
#include "core/math/Shapes.h"
#include "core/util/ResourcePath.h"
#include "core/types/Variant.h"

namespace oly::reg
{
	extern toml::v3::parse_result load_toml(const ResourcePath& file);

	extern bool parse_bool(TOMLNode node, bool& v);
	inline bool parse_bool_or(TOMLNode node, bool def) { parse_bool(node, def); return def; }
	extern bool parse_int(TOMLNode node, int& v);
	inline int parse_int_or(TOMLNode node, int def) { parse_int(node, def); return def; }
	extern bool parse_uint(TOMLNode node, unsigned int& v);
	inline unsigned int parse_uint_or(TOMLNode node, unsigned int def) { parse_uint(node, def); return def; }
	extern bool parse_float(TOMLNode node, float& v);
	inline float parse_float_or(TOMLNode node, float def) { parse_float(node, def); return def; }
	extern bool parse_double(TOMLNode node, double& v);
	inline double parse_double_or(TOMLNode node, double def) { parse_double(node, def); return def; }

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

	extern Transform2D load_transform_2d(TOMLNode node);

	extern bool parse_mag_filter(TOMLNode node, GLenum& mag_filter);
	extern bool parse_min_filter(TOMLNode node, GLenum& min_filter);
	extern bool parse_wrap(TOMLNode node, GLenum& wrap);

	namespace params
	{
		struct Transformer2D
		{
			Transform2D local;
			std::optional<Variant<ShearTransformModifier2D, PivotTransformModifier2D, OffsetTransformModifier2D>> modifier;
		};
	}

	extern Transformer2D load_transformer_2d(const params::Transformer2D& params);

	extern bool parse_shape(TOMLNode node, math::IRect2D& rect);

	extern math::TopSidePadding parse_topside_padding(TOMLNode node);
	extern math::Padding parse_padding(TOMLNode node);

	extern bool parse_enum(TOMLNode node, math::PositioningMode& mode);

	extern bool parse_color(const std::string& text, glm::vec4& color);
	extern bool parse_color(std::string&& text, glm::vec4& color);
}
