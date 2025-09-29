#pragma once

#include "registries/Loader.h"
#include "graphics/sprites/Sprite.h"

namespace oly::context
{
	namespace internal
	{
		extern void init_sprites(const TOMLNode&);
		extern void terminate_sprites();

		extern bool sprite_batch_is_rendering();
		extern void set_sprite_batch_rendering(bool ongoing);
	}

	extern rendering::SpriteBatch& sprite_batch();
	extern void render_sprites();
}
