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
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Parsing sprite atlas [" << (src ? *src: "") << "]." << LOG.nl;
		}

		params::SpriteAtlas params;

		params.sprite_params = sprite_params(node["sprite"]);

		int rows, cols;
		float delay_seconds;
		if (parse_int(node["rows"], rows) && parse_int(node["cols"], cols) && parse_float(node["delay_seconds"], delay_seconds))
		{
			params.frame = params::SpriteAtlas::Frame{
				.rows = (GLuint)rows,
				.cols = (GLuint)cols,
				.delay_seconds = delay_seconds,
				.row_major = parse_bool_or(node["row_major"], true),
				.row_up = parse_bool_or(node["row_up"], true)
			};
		}
		else
		{
			int static_frame;
			if (parse_int(node["static_frame"], static_frame))
				params.frame = params::SpriteAtlas::StaticFrame{ .frame = (GLuint)static_frame };
		}

		params.starting_frame = parse_int_or(node["starting_frame"], 0);
		params.starting_time = parse_float_or(node["starting_time"], 0.0f);

		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Sprite atlas [" << (src ? *src : "") << "] parsed." << LOG.nl;
		}

		return load_sprite_atlas(params);
	}

	rendering::SpriteAtlas load_sprite_atlas(const params::SpriteAtlas& params)
	{
		rendering::SpriteAtlas atlas(load_sprite(params.sprite_params));

		if (params.frame)
		{
			params.frame->visit(
				[&atlas](const params::SpriteAtlas::Frame& frame) { atlas.setup_uniform(frame.rows, frame.cols, frame.delay_seconds, frame.row_major, frame.row_up); },
				[&atlas](const params::SpriteAtlas::StaticFrame& frame) { atlas.select_static_frame(frame.frame); }
			);
		}
		atlas.anim_format.starting_frame = params.starting_frame;
		atlas.anim_format.starting_time = params.starting_time;

		return atlas;
	}
}
