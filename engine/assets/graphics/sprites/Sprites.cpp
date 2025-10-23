#include "Sprites.h"

#include "core/context/rendering/Textures.h"
#include "assets/Loader.h"

namespace oly::assets
{
	rendering::Sprite load_sprite(TOMLNode node)
	{
		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "ASSETS") << LOG.source_info.full_source() << "Parsing sprite [" << (src ? *src : "") << "]..." << LOG.nl;
		}

		rendering::Sprite sprite;
		sprite.transformer = load_transformer_2d(node["transformer"]);

		auto texture = node["texture"].value<std::string>();
		if (texture)
		{
			graphics::BindlessTextureRef btex = context::load_texture(*texture, parse_uint_or(node["texture_index"], 0));
			sprite.set_texture(btex, context::get_texture_dimensions(btex));
		}

		if (auto toml_modulation = node["modulation"])
		{
			glm::vec4 modulation;
			if (parse_vec(toml_modulation, modulation))
				sprite.set_modulation(modulation);
			else
				OLY_LOG_WARNING(true, "ASSETS") << LOG.source_info.full_source() << "Cannot parse \"modulation\" field." << LOG.nl;
		}

		if (auto toml_tex_coords = node["tex_coords"])
		{
			glm::vec4 uvs;
			if (parse_vec(toml_tex_coords, uvs))
				sprite.set_tex_coords({ .x1 = uvs[0], .x2 = uvs[1], .y1 = uvs[2], .y2 = uvs[3] });
			else
				OLY_LOG_WARNING(true, "ASSETS") << LOG.source_info.full_source() << "Cannot parse \"tex_coords\" field." << LOG.nl;
		}

		if (auto toml_frame_format = node["frame_format"])
		{
			if (auto mode = toml_frame_format["mode"].value<std::string>())
			{
				std::string mode_str = *mode;
				if (mode_str == "single")
				{
					if (texture)
						sprite.set_frame_format(graphics::setup_anim_frame_format_single(*texture, parse_uint_or(toml_frame_format["frame"], 0)));
					else
						OLY_LOG_WARNING(true, "ASSETS") << LOG.source_info.full_source() << "No texture was set for single frame format." << LOG.nl;
				}
				else if (mode_str == "auto")
				{
					if (texture)
						sprite.set_frame_format(graphics::setup_anim_frame_format(*texture, parse_float_or(toml_frame_format["speed"], 1.0f),
							parse_uint_or(toml_frame_format["starting_frame"], 0)));
					else
						OLY_LOG_WARNING(true, "ASSETS") << LOG.source_info.full_source() << "No texture was set for auto frame format." << LOG.nl;
				}
				else
					OLY_LOG_WARNING(true, "ASSETS") << LOG.source_info.full_source() << "Unrecognized frame format mode \"" << mode_str << "\"." << LOG.nl;
			}
			else
			{
				sprite.set_frame_format({
					.starting_frame = parse_uint_or(toml_frame_format["starting_frame"], 0),
					.num_frames = parse_uint_or(toml_frame_format["num_frames"], 0),
					.starting_time = parse_float_or(toml_frame_format["starting_time"], 0.0f),
					.delay_seconds = parse_float_or(toml_frame_format["delay_seconds"], 0.0f)
				});
			};
		}

		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "ASSETS") << LOG.source_info.full_source() << "...Sprite [" << (src ? *src : "") << "] parsed." << LOG.nl;
		}

		return sprite;
	}
}
