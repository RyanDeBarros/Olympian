#include "Loader.h"

#include "core/algorithms/STLUtils.h"
#include "core/base/Definitions.h"
#include "core/util/LoggerOperators.h"
#include "core/util/Parser.h"

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

	Polymorphic<TransformModifier2D> load_transform_modifier_2d(TOMLNode node)
	{
		if (auto mnode = io::Parser(node).optional<TOMLNode>(_gen::keys::Transform::Modifier)())
		{
			io::Parser parser(*mnode);

			if (auto type = parser.translate<_gen::TransformModifier>().optional(_gen::keys::Transform::ModifierType)())
			{
				switch (*type)
				{
				case TransformModifierType::None:
					break;
				case TransformModifierType::Shear:
				{
					Polymorphic<ShearTransformModifier2D> modifier;
					parser.optional(_gen::keys::Transform::Shearing)(modifier->shearing);
					return modifier;
				}
				case TransformModifierType::Pivot:
				{
					Polymorphic<PivotTransformModifier2D> modifier;
					parser.optional(_gen::keys::Transform::Pivot)(modifier->pivot);
					parser.optional(_gen::keys::Transform::Size)(modifier->size);
					return modifier;
				}
				case TransformModifierType::Offset:
				{
					Polymorphic<OffsetTransformModifier2D> modifier;
					parser.optional(_gen::keys::Transform::Offset)(modifier->offset);
					return modifier;
				}
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
