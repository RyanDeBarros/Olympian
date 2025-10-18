#include "Loader.h"

#include "external/GL.h"
#include "core/base/Errors.h"
#include "core/util/Logger.h"

namespace oly::reg
{
	toml::v3::parse_result load_toml(const char* file)
	{
		try
		{
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Loading TOML file \"" << file << "\"" << LOG.nl;
			return toml::parse_file(file);
		}
		catch (const toml::parse_error& err)
		{
			OLY_LOG_ERROR(true, "REG") << LOG.source_info.full_source() << "Cannot load TOML file \"" << file << "\"." << LOG.nl;
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

	Transform2D load_transform_2d(TOMLNode node)
	{
		Transform2D transform;
		if (!node)
			return transform;
		parse_vec(node["position"], transform.position);
		if (auto rotation = node["rotation"].value<double>())
			transform.rotation = (float)rotation.value();
		parse_vec(node["scale"], transform.scale);
		return transform;
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

	Transformer2D load_transformer_2d(const params::Transformer2D& params)
	{
		Transformer2D transformer;
		transformer.set_local() = params.local;
		if (params.modifier)
			transformer.set_modifier() = std::visit([](auto&& m) -> std::unique_ptr<TransformModifier2D> { return std::make_unique<std::decay_t<decltype(m)>>(m); }, params.modifier.value());
		return transformer;
	}

	bool parse_shape(TOMLNode node, math::IRect2D& rect)
	{
		if (auto x1 = node["x1"].value<int64_t>())
			rect.x1 = *x1;
		else
			return false;

		if (auto x2 = node["x2"].value<int64_t>())
			rect.x2 = *x2;
		else
			return false;

		if (auto y1 = node["y1"].value<int64_t>())
			rect.y1 = *y1;
		else
			return false;

		if (auto y2 = node["y2"].value<int64_t>())
			rect.y2 = *y2;
		else
			return false;

		return true;
	}

	math::TopSidePadding parse_topside_padding(TOMLNode node)
	{
		math::TopSidePadding padding;

		if (auto uniform = node["uniform"].value<double>())
			padding = math::TopSidePadding::uniform(*uniform);

		parse_float(node["left"], padding.left);
		parse_float(node["right"], padding.right);
		parse_float(node["top"], padding.top);

		return padding;
	}

	math::Padding parse_padding(TOMLNode node)
	{
		math::Padding padding;

		if (auto uniform = node["uniform"].value<double>())
			padding = math::Padding::uniform(*uniform);

		parse_float(node["left"], padding.left);
		parse_float(node["right"], padding.right);
		parse_float(node["top"], padding.top);
		parse_float(node["bottom"], padding.bottom);

		return padding;
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
}
