#pragma once

#include "external/GL.h"
#include "external/TOML.h"
#include "core/base/Transforms.h"
#include "core/math/Shapes.h"

namespace oly::reg
{
	extern toml::v3::parse_result load_toml(const char* file);
	inline toml::v3::parse_result load_toml(const std::string& file) { return load_toml(file.c_str()); }

	extern bool parse_int(const TOMLNode& node, const std::string& name, int& v);
	extern bool parse_int(const CTOMLNode& node, const std::string& name, int& v);
	extern bool parse_int(const toml::table& node, const std::string& name, int& v);
	extern bool parse_float(const TOMLNode& node, const std::string& name, float& v);
	extern bool parse_float(const CTOMLNode& node, const std::string& name, float& v);
	extern bool parse_float(const toml::table& node, const std::string& name, float& v);

	template<size_t N>
	inline bool parse_vec(const toml::v3::array* arr, glm::vec<N, float>& v)
	{
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
	inline bool parse_ivec(const toml::v3::array* arr, glm::vec<N, int>& v)
	{
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

	extern Transform2D load_transform_2d(const TOMLNode& node);
	extern Transform2D load_transform_2d(const CTOMLNode& node);
	inline Transform2D load_transform_2d(const toml::table& node, const char* name) { return load_transform_2d((const TOMLNode&)node[name]); }
	inline Transform2D load_transform_2d(const TOMLNode& node, const char* name) { return load_transform_2d(node[name]); }

	extern bool parse_mag_filter(const TOMLNode& node, const std::string& name, GLenum& mag_filter);
	extern bool parse_min_filter(const TOMLNode& node, const std::string& name, GLenum& min_filter);
	extern bool parse_wrap(const TOMLNode& node, const std::string& name, GLenum& wrap);

	namespace params
	{
		struct Transformer2D
		{
			Transform2D local;
			std::optional<std::variant<ShearTransformModifier2D, PivotTransformModifier2D, OffsetTransformModifier2D>> modifier;
		};
	}

	extern Transformer2D load_transformer_2d(const params::Transformer2D& params);

	extern bool parse_shape(const TOMLNode& node, math::IRect2D& rect);

	extern math::TopSidePadding parse_padding(const TOMLNode& node);

	extern bool parse_enum(const TOMLNode& node, math::PositioningMode& mode);
}
