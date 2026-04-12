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
	std::optional<bool> parse<bool>(TOMLNode node)
	{
		if (auto i = node.value<bool>())
			return *i;
		else if (auto i = node.value<int64_t>())
			return (bool)*i;
		else
			return std::nullopt;
	}

	template<>
	std::optional<int> parse<int>(TOMLNode node)
	{
		if (auto i = node.value<int64_t>())
			return (int)*i;
		else
			return std::nullopt;
	}

	template<>
	std::optional<unsigned int> parse<unsigned int>(TOMLNode node)
	{
		if (auto i = node.value<int64_t>())
			return (unsigned int)*i;
		else
			return std::nullopt;
	}

	template<>
	std::optional<float> parse<float>(TOMLNode node)
	{
		if (auto i = node.value<double>())
			return (float)*i;
		else if (auto i = node.value<int64_t>())
			return (float)*i;
		else
			return std::nullopt;
	}

	template<>
	std::optional<double> parse<double>(TOMLNode node)
	{
		if (auto i = node.value<double>())
			return *i;
		else if (auto i = node.value<int64_t>())
			return (double)*i;
		else
			return std::nullopt;
	}

	template<>
	std::optional<size_t> parse<size_t>(TOMLNode node)
	{
		if (auto i = node.value<int64_t>())
			return (size_t)*i;
		else
			return std::nullopt;
	}

	template<size_t N>
	std::optional<glm::vec<N, float>> parse_vec(TOMLNode node)
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
					return std::nullopt;
			}
			return u;
		}
		else
			return std::nullopt;
	}

	template<>
	std::optional<glm::vec2> parse<glm::vec2>(TOMLNode node)
	{
		return parse_vec<2>(node);
	}

	template<>
	std::optional<glm::vec3> parse<glm::vec3>(TOMLNode node)
	{
		return parse_vec<3>(node);
	}

	template<>
	std::optional<glm::vec4> parse<glm::vec4>(TOMLNode node)
	{
		return parse_vec<4>(node);
	}

	template<size_t N>
	std::optional<glm::vec<N, int>> parse_ivec(TOMLNode node)
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
					return std::nullopt;
			}
			return u;
		}
		else
			return std::nullopt;
	}

	template<>
	std::optional<glm::ivec2> parse<glm::ivec2>(TOMLNode node)
	{
		return parse_ivec<2>(node);
	}

	template<>
	std::optional<glm::ivec3> parse<glm::ivec3>(TOMLNode node)
	{
		return parse_ivec<3>(node);
	}

	template<>
	std::optional<glm::ivec4> parse<glm::ivec4>(TOMLNode node)
	{
		return parse_ivec<4>(node);
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
