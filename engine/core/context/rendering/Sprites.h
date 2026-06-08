#pragma once

#include "graphics/sprites/Sprite.h"

namespace oly::context
{
	namespace internal
	{
		extern void init_sprites();
	}

	extern rendering::SpriteBatch& sprite_batch();
	extern void render_sprites();
}
