#include "SpriteAtlas.h"

#include "core/util/Time.h"

namespace oly::rendering
{
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
			select(anim_format.starting_frame + (int)floor((TIME.now<float>() - anim_format.starting_time) / anim_format.delay_seconds));
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
		static const auto uv_rect = [](GLuint row, GLuint col, GLuint rows, GLuint cols) -> UVRect {
			return { {
				{       col / (float)cols,       row / (float)rows },
				{ (col + 1) / (float)cols,       row / (float)rows },
				{ (col + 1) / (float)cols, (row + 1) / (float)rows },
				{       col / (float)cols, (row + 1) / (float)rows }
			} };
			};
		if (row_major)
		{
			if (row_up)
			{
				for (GLuint row = 0; row < rows; ++row)
					for (GLuint col = 0; col < cols; ++col)
						atlas.push_back(uv_rect(row, col, rows, cols));
			}
			else
			{
				for (int row = rows - 1; row >= 0; --row)
					for (GLuint col = 0; col < cols; ++col)
						atlas.push_back(uv_rect(row, col, rows, cols));
			}
		}
		else
		{
			if (row_up)
			{
				for (GLuint col = 0; col < cols; ++col)
					for (GLuint row = 0; row < rows; ++row)
						atlas.push_back(uv_rect(row, col, rows, cols));
			}
			else
			{
				for (GLuint col = 0; col < cols; ++col)
					for (int row = rows - 1; row >= 0; --row)
						atlas.push_back(uv_rect(row, col, rows, cols));
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
}
