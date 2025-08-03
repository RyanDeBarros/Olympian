#pragma once

#include "registries/Loader.h"

namespace oly::rendering
{
	class SpriteBatch;
}

namespace oly::context
{
	namespace internal
	{
		extern void init_sprites(const TOMLNode&);
		extern void terminate_sprites();
	}

	extern rendering::SpriteBatch& sprite_batch();
	extern void render_sprites();
}
