#include "SpriteAtlases.h"

#include "registries/Loader.h"
#include "registries/graphics/sprites/Sprites.h"

namespace oly::reg
{
	rendering::SpriteAtlas load_sprite_atlas(TOMLNode node)
	{
		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Parsing sprite atlas [" << (src ? *src: "") << "]..." << LOG.nl;
		}

		rendering::SpriteAtlas sprite_atlas;
		if (auto sprite = node["sprite"])
			sprite_atlas = load_sprite(sprite);

		GLuint rows, cols;
		float delay_seconds;
		if (parse_uint(node["rows"], rows) && parse_uint(node["cols"], cols) && parse_float(node["delay_seconds"], delay_seconds))
			sprite_atlas.setup_uniform(rows, cols, delay_seconds, parse_bool_or(node["row_major"], true), parse_bool_or(node["row_up"], true));
		else
		{
			GLuint static_frame;
			if (parse_uint(node["static_frame"], static_frame))
				sprite_atlas.select_static_frame(static_frame);
		}

		sprite_atlas.anim_format.starting_frame = parse_int_or(node["starting_frame"], 0);
		sprite_atlas.anim_format.starting_time = parse_float_or(node["starting_time"], 0.0f);

		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "...Sprite atlas [" << (src ? *src : "") << "] parsed." << LOG.nl;
		}

		return sprite_atlas;
	}
}
