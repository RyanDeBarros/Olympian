#include "Sprites.h"

#include "core/context/rendering/Textures.h"
#include "registries/Loader.h"

namespace oly::reg
{
	params::Sprite sprite_params(TOMLNode node)
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
			if (auto mode = toml_frame_format["mode"].value<std::string>())
			{
				std::string mode_str = *mode;
				if (mode_str == "single")
					params.frame_format = params::Sprite::SingleFrameFormat{ .frame = parse_uint_or(toml_frame_format["frame"], 0) };
				else if (mode_str == "auto")
					params.frame_format = params::Sprite::AutoFrameFormat{
						.speed = parse_float_or(toml_frame_format["speed"], 1.0f),
						.starting_frame = parse_uint_or(toml_frame_format["starting_frame"], 0)
					};
				else
					OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Unrecognized frame format mode \"" << mode_str << "\"." << LOG.nl;
			}
			else
				params.frame_format = graphics::AnimFrameFormat{
					.starting_frame = parse_uint_or(toml_frame_format["starting_frame"], 0),
					.num_frames = parse_uint_or(toml_frame_format["num_frames"], 0),
					.starting_time = parse_float_or(toml_frame_format["starting_time"], 0.0f),
					.delay_seconds = parse_float_or(toml_frame_format["delay_seconds"], 0.0f)
				};
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

	rendering::Sprite load_sprite(TOMLNode node)
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
			sprite.set_texture(btex, context::get_texture_dimensions(btex));
		}

		if (params.modulation)
			sprite.set_modulation(*params.modulation);

		if (params.tex_coords)
			sprite.set_tex_coords(*params.tex_coords);

		if (params.frame_format)
		{
			const auto& frame_format = *params.frame_format;
			if (auto frame_format = params.frame_format->safe_get<graphics::AnimFrameFormat>())
				sprite.set_frame_format(*frame_format);
			else if (params.texture)
			{
				if (auto frame_format = params.frame_format->safe_get<params::Sprite::SingleFrameFormat>())
					sprite.set_frame_format(graphics::setup_anim_frame_format_single(*params.texture, frame_format->frame));
				else if (auto frame_format = params.frame_format->safe_get<params::Sprite::AutoFrameFormat>())
					sprite.set_frame_format(graphics::setup_anim_frame_format(*params.texture, frame_format->speed, frame_format->starting_frame));
			}
		}

		if (params.modifier)
			sprite.transformer.set_modifier() = params.modifier->visit([](const auto& m) -> std::unique_ptr<TransformModifier2D>
				{ return std::make_unique<std::decay_t<decltype(m)>>(m); });

		return sprite;
	}
}
