#include "SpriteAtlases.h"

#include "registries/graphics/primitives/Sprites.h"

namespace oly::reg
{
	rendering::SpriteAtlas load_sprite_atlas(const TOMLNode& node)
	{
		params::SpriteAtlas params;

		params.sprite_params = sprite_params(node["sprite"]);

		auto _rows = node["rows"].value<int64_t>();
		auto _cols = node["cols"].value<int64_t>();
		auto _delay_seconds = node["delay seconds"].value<double>();

		if (_rows && _cols && _delay_seconds)
		{
			params.frame = params::SpriteAtlas::Frame{
				.rows = (GLuint)_rows.value(),
				.cols = (GLuint)_cols.value(),
				.delay_seconds = (float)_delay_seconds.value(),
				.row_major = node["row major"].value<bool>().value_or(true),
				.row_up = node["row up"].value<bool>().value_or(true)
			};
		}
		else if (auto _static_frame = node["static frame"].value<int64_t>())
		{
			params.frame = params::SpriteAtlas::StaticFrame{ .frame = (GLuint)_static_frame.value() };
		}

		params.starting_frame = convert_optional<GLuint>(node["starting frame"].value<int64_t>());
		params.starting_time = convert_optional<float>(node["starting time"].value<double>());

		return load_sprite_atlas(params);
	}

	rendering::SpriteAtlas load_sprite_atlas(const params::SpriteAtlas& params)
	{
		rendering::SpriteAtlas atlas(load_sprite(params.sprite_params));

		if (params.frame)
		{
			std::visit([&atlas](auto&& frame) {
				if constexpr (std::is_same_v<std::decay_t<decltype(frame)>, params::SpriteAtlas::Frame>)
					atlas.setup_uniform(frame.rows, frame.cols, frame.delay_seconds, frame.row_major, frame.row_up);
				else if constexpr (std::is_same_v<std::decay_t<decltype(frame)>, params::SpriteAtlas::StaticFrame>)
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
