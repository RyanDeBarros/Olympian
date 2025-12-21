#pragma once

#include "graphics/sprites/Sprite.h"

namespace oly::context
{
	namespace internal
	{
		extern void init_sprites(TOMLNode);
		extern void terminate_sprites();
	}

	extern rendering::SpriteBatch& sprite_batch();
	extern void render_sprites();
}
