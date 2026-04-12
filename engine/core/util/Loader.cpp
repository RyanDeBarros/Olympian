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

	void parse_bool(TOMLNode node, std::optional<bool>& v)
	{
		if (auto i = node.value<bool>())
			v = *i;
		else if (auto i = node.value<int64_t>())
			v = (bool)*i;
		else
			v = std::nullopt;
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

	void parse_int(TOMLNode node, std::optional<int>& v)
	{
		if (auto i = node.value<int64_t>())
			v = (int)*i;
		else
			v = std::nullopt;
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

	void parse_uint(TOMLNode node, std::optional<unsigned int>& v)
	{
		if (auto i = node.value<int64_t>())
			v = (unsigned int)*i;
		else
			v = std::nullopt;
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

	void parse_float(TOMLNode node, std::optional<float>& v)
	{
		if (auto i = node.value<double>())
			v = (float)*i;
		else if (auto i = node.value<int64_t>())
			v = (float)*i;
		else
			v = std::nullopt;
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

	void parse_double(TOMLNode node, std::optional<double>& v)
	{
		if (auto i = node.value<double>())
			v = *i;
		else if (auto i = node.value<int64_t>())
			v = (double)*i;
		else
			v = std::nullopt;
	}

	bool parse_size_t(TOMLNode node, size_t& v)
	{
		if (auto i = node.value<int64_t>())
		{
			v = (size_t)*i;
			return true;
		}
		return false;
	}

	void parse_size_t(TOMLNode node, std::optional<size_t>& v)
	{
		if (auto i = node.value<int64_t>())
			v = (size_t)*i;
		else
			v = std::nullopt;
	}

	Polymorphic<TransformModifier2D> load_transform_modifier_2d(TOMLNode node)
	{
		if (auto type = io::parse_uint(io::parse_key(node, _gen::keys::Transform::ModifierType)))
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
					parse_vec(io::parse_key(node, _gen::keys::Transform::Shearing), modifier->shearing);
					return modifier;
				}
				case TransformModifierType::Pivot:
				{
					Polymorphic<PivotTransformModifier2D> modifier;
					parse_vec(io::parse_key(node, _gen::keys::Transform::Pivot), modifier->pivot);
					parse_vec(io::parse_key(node, _gen::keys::Transform::Size), modifier->size);
					return modifier;
				}
				case TransformModifierType::Offset:
				{
					Polymorphic<OffsetTransformModifier2D> modifier;
					parse_vec(io::parse_key(node, _gen::keys::Transform::Offset), modifier->offset);
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
