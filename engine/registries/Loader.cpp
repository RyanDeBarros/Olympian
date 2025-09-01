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
			return toml::parse_file(file);
		}
		catch (const toml::parse_error& err)
		{
			throw Error(ErrorCode::TOML_PARSE, err.description().data());
		}
	}

	toml::v3::parse_result load_toml(const std::string& file)
	{
		try
		{
			return toml::parse_file(file);
		}
		catch (const toml::parse_error& err)
		{
			OLY_LOG_ERROR(true, "REG") << LOG.source_info.full_source() << "Cannot load TOML file \"" << file << "\"." << LOG.nl;
			throw Error(ErrorCode::TOML_PARSE, err.description().data());
		}
	}

	bool parse_int(const TOMLNode& node, const std::string& name, int& v)
	{
		if (auto i = node[name].value<int64_t>())
		{
			v = (int)i.value();
			return true;
		}
		return false;
	}

	bool parse_int(const CTOMLNode& node, const std::string& name, int& v)
	{
		if (auto i = node[name].value<int64_t>())
		{
			v = (int)i.value();
			return true;
		}
		return false;
	}

	bool parse_int(const toml::table& node, const std::string& name, int& v)
	{
		if (auto i = node[name].value<int64_t>())
		{
			v = (int)i.value();
			return true;
		}
		return false;
	}

	bool parse_float(const TOMLNode& node, const std::string& name, float& v)
	{
		if (auto i = node[name].value<double>())
		{
			v = (float)i.value();
			return true;
		}
		return false;
	}

	bool parse_float(const CTOMLNode& node, const std::string& name, float& v)
	{
		if (auto i = node[name].value<double>())
		{
			v = (float)i.value();
			return true;
		}
		return false;
	}

	bool parse_float(const toml::table& node, const std::string& name, float& v)
	{
		if (auto i = node[name].value<double>())
		{
			v = (float)i.value();
			return true;
		}
		return false;
	}

	Transform2D load_transform_2d(const TOMLNode& node)
	{
		Transform2D transform;
		if (!node)
			return transform;
		parse_vec(node["position"].as_array(), transform.position);
		if (auto rotation = node["rotation"].value<double>())
			transform.rotation = (float)rotation.value();
		parse_vec(node["scale"].as_array(), transform.scale);
		return transform;
	}

	Transform2D load_transform_2d(const CTOMLNode& node)
	{
		Transform2D transform;
		if (!node)
			return transform;
		parse_vec(node["position"].as_array(), transform.position);
		if (auto rotation = node["rotation"].value<double>())
			transform.rotation = (float)rotation.value();
		parse_vec(node["scale"].as_array(), transform.scale);
		return transform;
	}

	bool parse_mag_filter(const TOMLNode& node, const std::string& name, GLenum& mag_filter)
	{
		if (auto v = node[name].value<int64_t>())
		{
			GLenum val = (GLenum)v.value();
			if (is_in(val, GL_NEAREST, GL_LINEAR))
			{
				mag_filter = val;
				return true;
			}
			else
				return false;
		}

		auto _v = node[name].value<std::string>();
		if (!_v)
			return false;
		const std::string& v = _v.value();
		if (v == "nearest")
			mag_filter = GL_NEAREST;
		else if (v == "linear")
			mag_filter = GL_LINEAR;
		else
			return false;
		return true;
	}

	bool parse_min_filter(const TOMLNode& node, const std::string& name, GLenum& min_filter)
	{
		if (auto v = node[name].value<int64_t>())
		{
			GLenum val = (GLenum)v.value();
			if (is_in(val, GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR))
			{
				min_filter = val;
				return true;
			}
			else
				return false;
		}

		auto _v = node[name].value<std::string>();
		if (!_v)
			return false;
		const std::string& v = _v.value();
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

	bool parse_wrap(const TOMLNode& node, const std::string& name, GLenum& wrap)
	{
		if (auto v = node[name].value<int64_t>())
		{
			GLenum val = (GLenum)v.value();
			if (is_in(val, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER, GL_MIRRORED_REPEAT, GL_REPEAT, GL_MIRROR_CLAMP_TO_EDGE))
			{
				wrap = val;
				return true;
			}
			else
				return false;
		}

		auto _v = node[name].value<std::string>();
		if (!_v)
			return false;
		const std::string& v = _v.value();
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
}
