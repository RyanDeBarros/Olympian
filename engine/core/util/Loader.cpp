#include "Loader.h"

#include "external/GL.h"
#include "core/base/Errors.h"
#include "core/util/LoggerOperators.h"
#include "core/algorithms/STLUtils.h"
#include "core/base/Definitions.h"

#include ".gen/keys/Transform.inl"

#include ".gen/enums/TransformModifier.inl"

// TODO v7 use CTOMLNode throughout

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
			throw Error(ErrorCode::TomlParse, err.description().data());
		}
	}

	template<>
	bool try_parse<bool>(TOMLNode node, bool& v)
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
		else
			return false;
	}

	template<>
	bool try_parse<int>(TOMLNode node, int& v)
	{
		if (auto i = node.value<int64_t>())
		{
			v = (int)*i;
			return true;
		}
		else
			return false;
	}

	template<>
	bool try_parse<unsigned int>(TOMLNode node, unsigned int& v)
	{
		if (auto i = node.value<int64_t>())
		{
			v = (unsigned int)*i;
			return true;
		}
		else
			return false;
	}

	template<>
	bool try_parse<float>(TOMLNode node, float& v)
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
		else
			return false;
	}

	template<>
	bool try_parse<double>(TOMLNode node, double& v)
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
		else
			return false;
	}

	template<>
	bool try_parse<size_t>(TOMLNode node, size_t& v)
	{
		if (auto i = node.value<int64_t>())
		{
			v = (size_t)*i;
			return true;
		}
		else
			return false;
	}

	template<size_t N, typename T, bool StrictSize>
	bool try_parse_vec(TOMLNode node, glm::vec<N, T>& v)
	{
		auto arr = node.as_array();
		if (arr && (StrictSize ? arr->size() == N : arr->size() <= N))
		{
			glm::vec<N, T> u = v;
			for (int i = 0; i < arr->size(); ++i)
				if (!try_parse((TOMLNode)*arr->get(i), u[i]))
					return false;
			v = u;
			return true;
		}
		else
			return false;
	}

	template<size_t N, typename T, bool StrictSize>
	bool try_parse_array(TOMLNode node, std::array<T, N>& v)
	{
		auto arr = node.as_array();
		if (arr && (StrictSize ? arr->size() == N : arr->size() <= N))
		{
			std::array<T, N> u = v;
			for (int i = 0; i < arr->size(); ++i)
				if (!try_parse((TOMLNode)*arr->get(i), u[i]))
					return false;
			v = u;
			return true;
		}
		else
			return false;
	}

	template<>
	bool try_parse<glm::vec2>(TOMLNode node, glm::vec2& v)
	{
		return try_parse_vec<2, float, true>(node, v);
	}

	template<>
	bool try_parse<glm::vec3>(TOMLNode node, glm::vec3& v)
	{
		return try_parse_vec<3, float, true>(node, v);
	}

	template<>
	bool try_parse<glm::vec4>(TOMLNode node, glm::vec4& v)
	{
		return try_parse_vec<4, float, true>(node, v);
	}

	template<>
	bool try_parse<glm::vec2>(TOMLNode node, PartialView<glm::vec2> v)
	{
		return try_parse_vec<2, float, false>(node, v.val);
	}

	template<>
	bool try_parse<glm::vec3>(TOMLNode node, PartialView<glm::vec3> v)
	{
		return try_parse_vec<3, float, false>(node, v.val);
	}

	template<>
	bool try_parse<glm::vec4>(TOMLNode node, PartialView<glm::vec4> v)
	{
		return try_parse_vec<4, float, false>(node, v.val);
	}

	template<>
	bool try_parse<glm::ivec2>(TOMLNode node, glm::ivec2& v)
	{
		return try_parse_vec<2, int, true>(node, v);
	}

	template<>
	bool try_parse<glm::ivec3>(TOMLNode node, glm::ivec3& v)
	{
		return try_parse_vec<3, int, true>(node, v);
	}

	template<>
	bool try_parse<glm::ivec4>(TOMLNode node, glm::ivec4& v)
	{
		return try_parse_vec<4, int, true>(node, v);
	}

	template<>
	bool try_parse<glm::ivec2>(TOMLNode node, PartialView<glm::ivec2> v)
	{
		return try_parse_vec<2, int, false>(node, v.val);
	}

	template<>
	bool try_parse<glm::ivec3>(TOMLNode node, PartialView<glm::ivec3> v)
	{
		return try_parse_vec<3, int, false>(node, v.val);
	}

	template<>
	bool try_parse<glm::ivec4>(TOMLNode node, PartialView<glm::ivec4> v)
	{
		return try_parse_vec<4, int, false>(node, v.val);
	}

	template<>
	bool try_parse<std::array<bool, 3>>(TOMLNode node, std::array<bool, 3>& v)
	{
		return try_parse_array<3, bool, true>(node, v);
	}

	template<>
	bool try_parse<std::array<bool, 3>>(TOMLNode node, PartialView<std::array<bool, 3>> v)
	{
		return try_parse_array<3, bool, false>(node, v.val);
	}

	template<>
	bool try_parse<std::string>(TOMLNode node, std::string& v)
	{
		if (auto s = node.value<std::string>())
		{
			v = *s;
			return true;
		}
		else
			return false;
	}

	template<>
	bool try_parse<TOMLArray>(TOMLNode node, TOMLArray& v)
	{
		if (auto o = node.as_array())
		{
			v = o;
			return true;
		}
		else
			return false;
	}

	namespace internal
	{
		void log_context_warning(const DeferredStringParam& msg)
		{
			_OLY_ENGINE_LOG_WARNING("CONTEXT") << msg.str() << LOG.nl;
		}

		void log_context_error(const DeferredStringParam& msg)
		{
			_OLY_ENGINE_LOG_ERROR("CONTEXT") << msg.str() << LOG.endl;
		}
	}

	Polymorphic<TransformModifier2D> load_transform_modifier_2d(TOMLNode node)
	{
		if (auto mnode = io::parse_key(node, _gen::keys::Transform::Modifier))
		{
			if (auto type = parse<unsigned int>(io::parse_key(mnode, _gen::keys::Transform::ModifierType)))
			{
				try
				{
					switch (_gen::TransformModifier::val(*type))
					{
					case TransformModifierType::None:
						break;
					case TransformModifierType::Shear:
					{
						Polymorphic<ShearTransformModifier2D> modifier;
						try_parse(io::parse_key(mnode, _gen::keys::Transform::Shearing), modifier->shearing);
						return modifier;
					}
					case TransformModifierType::Pivot:
					{
						Polymorphic<PivotTransformModifier2D> modifier;
						try_parse(io::parse_key(mnode, _gen::keys::Transform::Pivot), modifier->pivot);
						try_parse(io::parse_key(mnode, _gen::keys::Transform::Size), modifier->size);
						return modifier;
					}
					case TransformModifierType::Offset:
					{
						Polymorphic<OffsetTransformModifier2D> modifier;
						try_parse(io::parse_key(mnode, _gen::keys::Transform::Offset), modifier->offset);
						return modifier;
					}
					default:
						throw std::out_of_range("");
					}
				}
				catch (const std::out_of_range&)
				{
					_OLY_ENGINE_LOG_WARNING("ASSETS") << "Unrecognized transform modifier type (" << *type << ")" << LOG.nl;
				}
			}
		}
		return Polymorphic<TransformModifier2D>();
	}

	bool parse_color(const StringParam& text, glm::vec4& color)
	{
		text.to_lower();
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
