#include "SpriteAtlases.h"

#include "registries/Loader.h"
#include "registries/graphics/sprites/Sprites.h"

namespace oly::reg
{
	rendering::SpriteAtlas load_sprite_atlas(const TOMLNode& node)
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
			params::SpriteAtlas::Frame frame{
				.rows = (GLuint)rows,
				.cols = (GLuint)cols,
				.delay_seconds = delay_seconds,
			};
			parse_bool(node["row_major"], frame.row_major);
			parse_bool(node["row_up"], frame.row_up);
			params.frame = frame;
		}
		else
		{
			int static_frame;
			if (parse_int(node["static_frame"], static_frame))
				params.frame = params::SpriteAtlas::StaticFrame{ .frame = (GLuint)static_frame };
		}

		int starting_frame;
		if (parse_int(node["starting_frame"], starting_frame))
			params.starting_frame = starting_frame;

		float starting_time;
		if (parse_float(node["starting_time"], starting_time))
			params.starting_time = starting_time;

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
			std::visit([&atlas](auto&& frame) {
				if constexpr (visiting_class_is<decltype(frame), params::SpriteAtlas::Frame>)
					atlas.setup_uniform(frame.rows, frame.cols, frame.delay_seconds, frame.row_major, frame.row_up);
				else if constexpr (visiting_class_is<decltype(frame), params::SpriteAtlas::StaticFrame>)
					atlas.select_static_frame(frame.frame);
				}, params.frame.value());
		}
		if (params.starting_frame)
			atlas.anim_format.starting_frame = params.starting_frame.value();
		if (params.starting_time)
			atlas.anim_format.starting_time = params.starting_time.value();

		return atlas;
	}
}
