#include "SpriteAtlases.h"

#include "registries/graphics/primitives/Sprites.h"

namespace oly::reg
{
	rendering::SpriteAtlasExtension load_sprite_atlas(const TOMLNode& node)
	{
		rendering::SpriteAtlasExtension atlas(load_sprite(node["sprite"]));

		auto _rows = node["rows"].value<int64_t>();
		auto _cols = node["cols"].value<int64_t>();
		auto _delay_seconds = node["delay seconds"].value<double>();

		if (_rows && _cols && _delay_seconds)
		{
			auto row_major = node["row major"].value<bool>().value_or(true);
			auto row_up = node["row up"].value<bool>().value_or(true);
			atlas.setup_uniform((GLuint)_rows.value(), (GLuint)_cols.value(), (float)_delay_seconds.value(), row_major, row_up);

			if (auto _starting_frame = node["starting frame"].value<int64_t>())
				atlas.anim_format.starting_frame = (GLuint)_starting_frame.value();
			if (auto _starting_time = node["starting time"].value<double>())
				atlas.anim_format.starting_time = (float)_starting_time.value();
			if (auto _static_frame = node["static frame"].value<int64_t>()) // TODO this should be in else clause if rows/cols/delay_cs are not provided
				atlas.select_static_frame((GLuint)_static_frame.value());

			atlas.on_tick();
		}

		return atlas;
	}
}
