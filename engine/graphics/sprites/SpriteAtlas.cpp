#include "SpriteAtlas.h"

#include "core/util/Time.h"
#include "assets/Loader.h"

namespace oly::rendering
{
	SpriteAtlas::SpriteAtlas(Unbatched)
		: sprite(UNBATCHED)
	{
	}

	SpriteAtlas::SpriteAtlas(SpriteBatch& batch)
		: sprite(batch)
	{
	}

	SpriteAtlas::SpriteAtlas(Sprite&& sprite)
		: sprite(std::move(sprite))
	{
	}

	void SpriteAtlas::draw() const
	{
		sprite.draw();
	}

	void SpriteAtlas::on_tick() const
	{
		if (anim_format.delay_seconds != 0.0f)
			select(anim_format.starting_frame + (int)floor((TIME.now<>() - anim_format.starting_time) / anim_format.delay_seconds));
	}

	void SpriteAtlas::select_static_frame(GLuint frame)
	{
		anim_format.delay_seconds = 0.0f;
		select(frame);
	}

	void SpriteAtlas::uvs_changed() const
	{
		current_frame = -1;
	}

	void SpriteAtlas::setup_uniform(GLuint rows, GLuint cols, float delay_seconds, bool row_major, bool row_up)
	{
		atlas.clear();
		if (row_major)
		{
			if (row_up)
			{
				for (GLuint row = 0; row < rows; ++row)
					for (GLuint col = 0; col < cols; ++col)
						atlas.push_back(math::UVRect::from_grid(row, col, rows, cols));
			}
			else
			{
				for (int row = rows - 1; row >= 0; --row)
					for (GLuint col = 0; col < cols; ++col)
						atlas.push_back(math::UVRect::from_grid(row, col, rows, cols));
			}
		}
		else
		{
			if (row_up)
			{
				for (GLuint col = 0; col < cols; ++col)
					for (GLuint row = 0; row < rows; ++row)
						atlas.push_back(math::UVRect::from_grid(row, col, rows, cols));
			}
			else
			{
				for (GLuint col = 0; col < cols; ++col)
					for (int row = rows - 1; row >= 0; --row)
						atlas.push_back(math::UVRect::from_grid(row, col, rows, cols));
			}
		}
		uvs_changed();
		anim_format.num_frames = rows * cols;
		anim_format.delay_seconds = delay_seconds;
		anim_format.starting_frame = 0;
		anim_format.starting_time = 0.0f;

		glm::vec2 dimensions;
		auto texture = sprite.get_texture(dimensions);
		sprite.set_texture(texture, dimensions * glm::vec2{ 1.0f / cols, 1.0f / rows });
	}

	void SpriteAtlas::select(GLuint frame) const
	{
		frame %= anim_format.num_frames;
		if (current_frame != frame)
		{
			current_frame = frame;
			sprite.set_tex_coords(atlas[frame]);
		}
	}

	SpriteAtlas SpriteAtlas::load(TOMLNode node, const char* source)
	{
		if (!node)
			return {};

		_OLY_ENGINE_LOG_DEBUG("ASSETS") << "Parsing sprite atlas [" << (source ? source : "") << "]..." << LOG.nl;

		SpriteAtlas sprite_atlas(Sprite::load(node["sprite"], source));

		GLuint rows, cols;
		float delay_seconds;
		if (assets::parse_uint(node["rows"], rows) && assets::parse_uint(node["cols"], cols) && assets::parse_float(node["delay_seconds"], delay_seconds))
			sprite_atlas.setup_uniform(rows, cols, delay_seconds, assets::parse_bool_or(node["row_major"], true), assets::parse_bool_or(node["row_up"], true));
		else
		{
			GLuint static_frame;
			if (assets::parse_uint(node["static_frame"], static_frame))
				sprite_atlas.select_static_frame(static_frame);
		}

		sprite_atlas.anim_format.starting_frame = assets::parse_int_or(node["starting_frame"], 0);
		sprite_atlas.anim_format.starting_time = assets::parse_float_or(node["starting_time"], 0.0f);

		_OLY_ENGINE_LOG_DEBUG("ASSETS") << "...Sprite atlas [" << (source ? source : "") << "] parsed." << LOG.nl;

		return sprite_atlas;
	}
}
