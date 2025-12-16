#include "Loader.h"

#include "external/GL.h"
#include "core/base/Errors.h"
#include "core/util/LoggerOperators.h"
#include "core/algorithms/STLUtils.h"

namespace oly::io
{
	toml::v3::parse_result load_toml(const ResourcePath& file)
	{
		try
		{
			_OLY_ENGINE_LOG_DEBUG("ASSETS") << "Loading TOML file " << file << LOG.nl;
			return toml::parse_file(file.get_absolute().c_str());
		}
		catch (const toml::parse_error& err)
		{
			_OLY_ENGINE_LOG_ERROR("ASSETS") << "Cannot load TOML file " << file << LOG.nl;
			throw Error(ErrorCode::TOML_PARSE, err.description().data());
		}
	}

	bool parse_bool(TOMLNode node, bool& v)
	{
		if (auto i = node.value<bool>())
		{
			v = *i;
			return true;
		}
		else if (auto i = node.value<int64_t>())
		{
			v = (bool)*i;
			return true;
		}
		return false;
	}

	bool parse_int(TOMLNode node, int& v)
	{
		if (auto i = node.value<int64_t>())
		{
			v = (int)*i;
			return true;
		}
		return false;
	}

	bool parse_uint(TOMLNode node, unsigned int& v)
	{
		if (auto i = node.value<int64_t>())
		{
			v = (unsigned int)*i;
			return true;
		}
		return false;
	}

	bool parse_float(TOMLNode node, float& v)
	{
		if (auto i = node.value<double>())
		{
			v = (float)*i;
			return true;
		}
		else if (auto i = node.value<int64_t>())
		{
			v = (float)*i;
			return true;
		}
		return false;
	}

	bool parse_double(TOMLNode node, double& v)
	{
		if (auto i = node.value<double>())
		{
			v = *i;
			return true;
		}
		else if (auto i = node.value<int64_t>())
		{
			v = (double)*i;
			return true;
		}
		return false;
	}

	bool parse_mag_filter(TOMLNode node, GLenum& mag_filter)
	{
		if (auto v = node.value<int64_t>())
		{
			GLenum val = (GLenum)*v;
			if (is_in(val, GL_NEAREST, GL_LINEAR))
			{
				mag_filter = val;
				return true;
			}
			else
				return false;
		}

		auto _v = node.value<std::string>();
		if (!_v)
			return false;
		const std::string& v = *_v;
		if (v == "nearest")
			mag_filter = GL_NEAREST;
		else if (v == "linear")
			mag_filter = GL_LINEAR;
		else
			return false;
		return true;
	}

	bool parse_min_filter(TOMLNode node, GLenum& min_filter)
	{
		if (auto v = node.value<int64_t>())
		{
			GLenum val = (GLenum)*v;
			if (is_in(val, GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR))
			{
				min_filter = val;
				return true;
			}
			else
				return false;
		}

		auto _v = node.value<std::string>();
		if (!_v)
			return false;
		const std::string& v = *_v;
		if (v == "nearest")
			min_filter = GL_NEAREST;
		else if (v == "linear")
			min_filter = GL_LINEAR;
		else if (v == "nearest mipmap nearest")
			min_filter = GL_NEAREST_MIPMAP_NEAREST;
		else if (v == "nearest mipmap linear")
			min_filter = GL_NEAREST_MIPMAP_LINEAR;
		else if (v == "linear mipmap nearest")
			min_filter = GL_LINEAR_MIPMAP_NEAREST;
		else if (v == "linear mipmap linear")
			min_filter = GL_LINEAR_MIPMAP_LINEAR;
		else
			return false;
		return true;
	}

	bool parse_wrap(TOMLNode node, GLenum& wrap)
	{
		if (auto v = node.value<int64_t>())
		{
			GLenum val = (GLenum)*v;
			if (is_in(val, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER, GL_MIRRORED_REPEAT, GL_REPEAT, GL_MIRROR_CLAMP_TO_EDGE))
			{
				wrap = val;
				return true;
			}
			else
				return false;
		}

		auto _v = node.value<std::string>();
		if (!_v)
			return false;
		const std::string& v = *_v;
		if (v == "clamp to edge")
			wrap = GL_CLAMP_TO_EDGE;
		else if (v == "clamp to border")
			wrap = GL_CLAMP_TO_BORDER;
		else if (v == "mirrored repeat")
			wrap = GL_MIRRORED_REPEAT;
		else if (v == "repeat")
			wrap = GL_REPEAT;
		else if (v == "mirror clamp to edge")
			wrap = GL_MIRROR_CLAMP_TO_EDGE;
		else
			return false;
		return true;
	}

	Polymorphic<TransformModifier2D> load_transform_modifier_2d(TOMLNode node)
	{
		if (auto toml_type = node["type"].value<std::string>())
		{
			const std::string& type = *toml_type;
			if (type == "shear")
			{
				Polymorphic<ShearTransformModifier2D> modifier;
				parse_vec(node["shearing"], modifier->shearing);
				return modifier;
			}
			else if (type == "pivot")
			{
				Polymorphic<PivotTransformModifier2D> modifier;
				parse_vec(node["pivot"], modifier->pivot);
				parse_vec(node["size"], modifier->size);
				return modifier;
			}
			else if (type == "offset")
			{
				Polymorphic<OffsetTransformModifier2D> modifier;
				parse_vec(node["offset"], modifier->offset);
				return modifier;
			}
			else
				_OLY_ENGINE_LOG_WARNING("ASSETS") << "Unrecognized transform modifier type \"" << type << "\"." << LOG.nl;
		}
		return Polymorphic<TransformModifier2D>();
	}

	bool parse_enum(TOMLNode node, math::PositioningMode& mode)
	{
		auto _s = node.value<std::string>();
		if (!_s)
		{
			int v = 0;
			if (parse_int(node, v))
			{
				math::PositioningMode m = (math::PositioningMode)v;
				if (is_in(m, math::PositioningMode::ABSOLUTE, math::PositioningMode::RELATIVE))
				{
					mode = m;
					return true;
				}
			}
			return false;
		}
		const std::string& s = *_s;
		
		if (s == "absolute")
			mode = math::PositioningMode::ABSOLUTE;
		else if (s == "relative")
			mode = math::PositioningMode::RELATIVE;
		else
			return false;
		return true;
	}

	bool parse_color(const std::string& text, glm::vec4& color)
	{
		return parse_color(dupl(text), color);
	}

	bool parse_color(std::string&& text, glm::vec4& color)
	{
		algo::to_lower(text);
		if (text == "red")
		{
			color = { 1.0f, 0.0f, 0.0f, 1.0f };
			return true;
		}
		else if (text == "green")
		{
			color = { 0.0f, 1.0f, 0.0f, 1.0f };
			return true;
		}
		else if (text == "blue")
		{
			color = { 0.0f, 0.0f, 1.0f, 1.0f };
			return true;
		}
		else if (text == "cyan")
		{
			color = { 0.0f, 1.0f, 1.0f, 1.0f };
			return true;
		}
		else if (text == "magenta")
		{
			color = { 1.0f, 0.0f, 1.0f, 1.0f };
			return true;
		}
		else if (text == "yellow")
		{
			color = { 1.0f, 1.0f, 0.0f, 1.0f };
			return true;
		}
		else
			return false;
	}
}
