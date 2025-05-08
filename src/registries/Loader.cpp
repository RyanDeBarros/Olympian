#include "Loader.h"

#include "external/GL.h"
#include "core/base/Errors.h"

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
			throw Error(ErrorCode::TOML_PARSE, err.description().data());
		}
	}

	bool parse_int(const TOMLNode& node, const std::string& name, int& v)
	{
		auto n = node[name];
		if (n)
		{
			auto i = n.value<int64_t>();
			if (i)
			{
				v = (int)i.value();
				return true;
			}
		}
		return false;
	}

	bool parse_int(const CTOMLNode& node, const std::string& name, int& v)
	{
		auto n = node[name];
		if (n)
		{
			auto i = n.value<int64_t>();
			if (i)
			{
				v = (int)i.value();
				return true;
			}
		}
		return false;
	}

	bool parse_int(const toml::table& node, const std::string& name, int& v)
	{
		auto n = node[name];
		if (n)
		{
			auto i = n.value<int64_t>();
			if (i)
			{
				v = (int)i.value();
				return true;
			}
		}
		return false;
	}

	bool parse_float(const TOMLNode& node, const std::string& name, float& v)
	{
		auto n = node[name];
		if (n)
		{
			auto i = n.value<double>();
			if (i)
			{
				v = (float)i.value();
				return true;
			}
		}
		return false;
	}

	bool parse_float(const CTOMLNode& node, const std::string& name, float& v)
	{
		auto n = node[name];
		if (n)
		{
			auto i = n.value<double>();
			if (i)
			{
				v = (float)i.value();
				return true;
			}
		}
		return false;
	}

	bool parse_float(const toml::table& node, const std::string& name, float& v)
	{
		auto n = node[name];
		if (n)
		{
			auto i = n.value<double>();
			if (i)
			{
				v = (float)i.value();
				return true;
			}
		}
		return false;
	}

	template<size_t N>
	static bool parse_vec(const toml::v3::array* arr, glm::vec<N, float>& v)
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

	bool parse_vec2(const toml::v3::array* arr, glm::vec2& v)
	{
		return parse_vec(arr, v);
	}

	bool parse_vec4(const toml::v3::array* arr, glm::vec4& v)
	{
		return parse_vec(arr, v);
	}

	Transform2D load_transform_2d(const TOMLNode& node)
	{
		Transform2D transform;
		if (!node)
			return transform;
		parse_vec2(node, "position", transform.position);
		if (auto rotation = node["rotation"].value<double>())
			transform.rotation = (float)rotation.value();
		parse_vec2(node, "scale", transform.scale);
		return transform;
	}

	Transform2D load_transform_2d(const CTOMLNode& node)
	{
		Transform2D transform;
		if (!node)
			return transform;
		parse_vec2(node, "position", transform.position);
		if (auto rotation = node["rotation"].value<double>())
			transform.rotation = (float)rotation.value();
		parse_vec2(node, "scale", transform.scale);
		return transform;
	}

	bool parse_mag_filter(const TOMLNode& node, const std::string& name, GLenum& mag_filter)
	{
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

	bool parse_mag_filter(const toml::table& node, const std::string& name, GLenum& mag_filter)
	{
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

	bool parse_min_filter(const toml::table& node, const std::string& name, GLenum& min_filter)
	{
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

	bool parse_wrap(const toml::table& node, const std::string& name, GLenum& wrap)
	{
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
}
