#pragma once

#include "external/GL.h"
#include "external/TOML.h"
#include "core/base/Transforms.h"
#include "core/math/Shapes.h"

namespace oly::reg
{
	extern toml::v3::parse_result load_toml(const char* file);
	inline toml::v3::parse_result load_toml(const std::string& file) { return load_toml(file.c_str()); }

	extern bool parse_bool(TOMLNode node, bool& v);
	extern bool parse_int(TOMLNode node, int& v);
	extern bool parse_uint(TOMLNode node, GLuint& v);
	extern bool parse_float(TOMLNode node, float& v);
	extern bool parse_double(TOMLNode node, double& v);

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
			std::optional<std::variant<ShearTransformModifier2D, PivotTransformModifier2D, OffsetTransformModifier2D>> modifier;
		};
	}

	extern Transformer2D load_transformer_2d(const params::Transformer2D& params);

	extern bool parse_shape(TOMLNode node, math::IRect2D& rect);

	extern math::TopSidePadding parse_padding(TOMLNode node);

	extern bool parse_enum(TOMLNode node, math::PositioningMode& mode);
}
