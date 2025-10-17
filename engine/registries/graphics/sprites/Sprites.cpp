#include "Sprites.h"

#include "core/context/rendering/Textures.h"
#include "registries/Loader.h"
#include "registries/graphics/TextureRegistry.h"

namespace oly::reg
{
	params::Sprite sprite_params(const TOMLNode& node)
	{
		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Parsing sprite [" << (src ? *src : "") << "]." << LOG.nl;
		}

		params::Sprite params;

		params.local = load_transform_2d(node["transform"]);
		params.texture = node["texture"].value<std::string>();
		parse_uint(node["texture_index"], params.texture_index);

		if (auto toml_modulation = node["modulation"])
		{
			glm::vec4 modulation;
			if (parse_vec(toml_modulation, modulation))
				params.modulation = modulation;
			else
				OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Cannot parse \"modulation\" field." << LOG.nl;
		}

		if (auto toml_tex_coords = node["tex_coords"])
		{
			glm::vec4 uvs;
			if (parse_vec(toml_tex_coords, uvs))
				params.tex_coords = { .x1 = uvs[0], .x2 = uvs[1], .y1 = uvs[2], .y2 = uvs[3] };
			else
				OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Cannot parse \"tex_coords\" field." << LOG.nl;
		}

		if (auto toml_frame_format = node["frame_format"])
		{
			auto mode = toml_frame_format["mode"].value<std::string>();
			if (mode)
			{
				std::string mode_str = *mode;
				if (mode_str == "single")
				{
					params::Sprite::SingleFrameFormat format;
					parse_uint(toml_frame_format["frame"], format.frame);
					params.frame_format = format;
				}
				else if (mode_str == "auto")
				{
					params::Sprite::AutoFrameFormat format;
					parse_float(toml_frame_format["speed"], format.speed);
					parse_uint(toml_frame_format["starting_frame"], format.starting_frame);
					params.frame_format = format;
				}
				else
					OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Unrecognized frame format mode \"" << mode_str << "\"." << LOG.nl;
			}
			else
			{
				graphics::AnimFrameFormat format;
				parse_uint(toml_frame_format["starting_frame"], format.starting_frame);
				parse_uint(toml_frame_format["num_frames"], format.num_frames);
				parse_float(toml_frame_format["starting_time"], format.starting_time);
				parse_float(toml_frame_format["delay_seconds"], format.delay_seconds);
				params.frame_format = format;
			}
		}

		if (auto toml_transformer_modifier = node["transform_modifier"])
		{
			auto toml_type = toml_transformer_modifier["type"].value<std::string>();
			if (toml_type)
			{
				std::string type = *toml_type;
				if (type == "shear")
				{
					ShearTransformModifier2D modifier;
					parse_vec(node["shearing"], modifier.shearing);
					params.modifier = modifier;
				}
				else if (type == "pivot")
				{
					PivotTransformModifier2D modifier;
					parse_vec(node["pivot"], modifier.pivot);
					parse_vec(node["size"], modifier.size);
					params.modifier = modifier;
				}
				else if (type == "pivot-shear")
				{
					OffsetTransformModifier2D modifier;
					parse_vec(node["offset"], modifier.offset);
					params.modifier = modifier;
				}
				else
					OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Unrecognized transform modifier type \"" << type << "\"." << LOG.nl;
			}
		}

		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Sprite [" << (src ? *src : "") << "] parsed." << LOG.nl;
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
			graphics::BindlessTextureRef btex = context::load_texture(*params.texture, params.texture_index);
			sprite.set_texture(btex, context::get_texture_dimensions(*params.texture, params.texture_index));
		}

		if (params.modulation)
			sprite.set_modulation(*params.modulation);

		if (params.tex_coords)
			sprite.set_tex_coords(*params.tex_coords);

		if (params.frame_format)
		{
			const auto& frame_format = *params.frame_format;
			if (frame_format.index() == params::Sprite::FrameFormatIndex::CUSTOM)
				sprite.set_frame_format(std::get<params::Sprite::FrameFormatIndex::CUSTOM>(frame_format));
			else if (params.texture)
			{
				if (frame_format.index() == params::Sprite::FrameFormatIndex::SINGLE)
					sprite.set_frame_format(graphics::setup_anim_frame_format_single(*params.texture, std::get<params::Sprite::FrameFormatIndex::SINGLE>(frame_format).frame));
				else if (frame_format.index() == params::Sprite::FrameFormatIndex::AUTO)
					sprite.set_frame_format(graphics::setup_anim_frame_format(*params.texture, std::get<params::Sprite::FrameFormatIndex::AUTO>(frame_format).speed,
						std::get<params::Sprite::FrameFormatIndex::AUTO>(frame_format).starting_frame));
			}
		}

		if (params.modifier)
			sprite.transformer.set_modifier() = std::visit([](auto&& m) -> std::unique_ptr<TransformModifier2D>
				{ return std::make_unique<std::decay_t<decltype(m)>>(m); }, *params.modifier);

		return sprite;
	}
}
