#include "Sprites.h"

#include "core/base/Context.h"
#include "registries/Loader.h"
#include "registries/graphics/TextureRegistry.h"

namespace oly::reg
{
	params::Sprite sprite_params(const TOMLNode& node)
	{
		params::Sprite params;

		params.local = load_transform_2d(node, "transform");
		params.texture = node["texture"].value<std::string>();
		params.texture_index = (unsigned int)node["texture index"].value<int64_t>().value_or(0);
		params.svg_scale = convert_optional<float>(node["svg scale"].value<double>());

		if (auto toml_modulation = node["modulation"].as_array())
		{
			if (toml_modulation->size() == 4)
			{
				if (toml_modulation->get(0)->is_array())
				{
					rendering::ModulationRect modulation;
					if (parse_vec(toml_modulation->get_as<toml::array>(0), modulation.colors[0])
						&& parse_vec(toml_modulation->get_as<toml::array>(1), modulation.colors[1])
						&& parse_vec(toml_modulation->get_as<toml::array>(2), modulation.colors[2])
						&& parse_vec(toml_modulation->get_as<toml::array>(3), modulation.colors[3]))
						params.modulation = modulation;
				}
				else
				{
					glm::vec4 modulation;
					if (parse_vec(toml_modulation, modulation))
						params.modulation = modulation;
				}
			}
		}

		if (auto toml_tex_coords = node["tex coords"].as_array())
		{
			if (toml_tex_coords->size() == 4)
			{
				rendering::UVRect uvs;
				if (parse_vec(toml_tex_coords->get_as<toml::array>(0), uvs.uvs[0])
					&& parse_vec(toml_tex_coords->get_as<toml::array>(1), uvs.uvs[1])
					&& parse_vec(toml_tex_coords->get_as<toml::array>(2), uvs.uvs[2])
					&& parse_vec(toml_tex_coords->get_as<toml::array>(3), uvs.uvs[3]))
					params.tex_coords = uvs;
			}
		}

		if (auto toml_frame_format = node["frame_format"])
		{
			auto mode = toml_frame_format["mode"].value<std::string>();
			if (mode)
			{
				if (mode == "single")
					params.frame_format = params::Sprite::SingleFrameFormat{ .frame = (GLuint)toml_frame_format["frame"].value<int64_t>().value_or(0) };
				else if (mode == "auto")
					params.frame_format = params::Sprite::AutoFrameFormat{ .speed = (float)toml_frame_format["speed"].value<double>().value_or(1.0),
						.starting_frame = (GLuint)toml_frame_format["starting frame"].value<int64_t>().value_or(0) };
			}
			else
			{
				params.frame_format = graphics::AnimFrameFormat{
					.starting_frame = (GLuint)toml_frame_format["starting frame"].value<int64_t>().value_or(0),
					.num_frames = (GLuint)toml_frame_format["num frames"].value<int64_t>().value_or(0),
					.starting_time = (float)toml_frame_format["starting time"].value<double>().value_or(0.0),
					.delay_seconds = (float)toml_frame_format["delay seconds"].value<double>().value_or(0.0)
				};
			}
		}

		if (auto toml_transformer_modifier = node["transform_modifier"])
		{
			auto toml_type = toml_transformer_modifier["type"].value<std::string>();
			if (toml_type)
			{
				std::string type = toml_type.value();
				if (type == "shear")
				{
					ShearTransformModifier2D modifier;
					parse_vec(node["shearing"].as_array(), modifier.shearing);
					params.modifier = modifier;
				}
				else if (type == "pivot")
				{
					PivotTransformModifier2D modifier;
					parse_vec(node["pivot"].as_array(), modifier.pivot);
					parse_vec(node["size"].as_array(), modifier.size);
					params.modifier = modifier;
				}
				else if (type == "pivot-shear")
				{
					OffsetTransformModifier2D modifier;
					parse_vec(node["offset"].as_array(), modifier.offset);
					params.modifier = modifier;
				}
			}
		}

		return params;
	}

	rendering::Sprite load_sprite(const TOMLNode& node)
	{

		return load_sprite(sprite_params(node));
	}

	rendering::Sprite load_sprite(const params::Sprite& params)
	{
		rendering::Sprite sprite;
		sprite.set_local() = params.local;

		if (params.texture)
		{
			if (params.svg_scale)
			{
				reg::TextureRegistry::SVGLoadParams lparams{ .texture_index = params.texture_index };
				graphics::BindlessTextureRef btex = context::texture_registry().load_svg_texture(params.texture.value(), params.svg_scale.value(), lparams);
				sprite.set_texture(btex, context::texture_registry().get_dimensions(params.texture.value(), params.texture_index));
			}
			else
			{
				reg::TextureRegistry::LoadParams lparams{ .texture_index = params.texture_index };
				graphics::BindlessTextureRef btex = context::texture_registry().load_texture(params.texture.value(), lparams);
				sprite.set_texture(btex, context::texture_registry().get_dimensions(params.texture.value(), params.texture_index));
			}
		}

		if (params.modulation)
			std::visit([&sprite](auto&& m) { return sprite.set_modulation(m); }, params.modulation.value());

		if (params.tex_coords)
			sprite.set_tex_coords(params.tex_coords.value());

		if (params.frame_format)
		{
			const auto& frame_format = params.frame_format.value();
			if (frame_format.index() == params::Sprite::FrameFormatIndex::CUSTOM)
				sprite.set_frame_format(std::get<params::Sprite::FrameFormatIndex::CUSTOM>(frame_format));
			else if (params.texture)
			{
				if (frame_format.index() == params::Sprite::FrameFormatIndex::SINGLE)
					sprite.set_frame_format(graphics::setup_anim_frame_format_single(params.texture.value(), std::get<params::Sprite::FrameFormatIndex::SINGLE>(frame_format).frame));
				else if (frame_format.index() == params::Sprite::FrameFormatIndex::AUTO)
					sprite.set_frame_format(graphics::setup_anim_frame_format(params.texture.value(), std::get<params::Sprite::FrameFormatIndex::AUTO>(frame_format).speed,
						std::get<params::Sprite::FrameFormatIndex::AUTO>(frame_format).starting_frame));
			}
		}

		if (params.modifier)
			sprite.transformer.set_modifier() = std::visit([](auto&& m) -> std::unique_ptr<TransformModifier2D> { return std::make_unique<std::decay_t<decltype(m)>>(m); }, params.modifier.value());

		return sprite;
	}
}
