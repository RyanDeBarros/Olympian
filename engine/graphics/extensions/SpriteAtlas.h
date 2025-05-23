#pragma once

#include "graphics/primitives/Sprites.h"

namespace oly::rendering
{
	struct SpriteAtlas
	{
		Sprite sprite;
		UVAtlas atlas;
		graphics::AnimFrameFormat anim_format;

		SpriteAtlas(Sprite&& sprite);

		void draw() const;
		void on_tick() const;

		void select_static_frame(GLuint frame);
		void uvs_changed() const;
		void setup_uniform(GLuint rows, GLuint cols, float delay_seconds, bool row_major = true, bool row_up = true);

	private:
		void select(GLuint frame) const;
		mutable GLuint current_frame = -1;
	};
}
