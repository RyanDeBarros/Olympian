#include "Sprites.h"

#include "core/context/rendering/Rendering.h"
#include "graphics/sprites/Sprite.h"

namespace oly::context
{
	namespace internal
	{
		std::unique_ptr<rendering::SpriteBatch> sprite_batch;
		bool sprite_batch_rendering = false;
	}

	void internal::init_sprites(const TOMLNode& node)
	{
		internal::sprite_batch = std::make_unique<rendering::SpriteBatch>();
	}

	void internal::terminate_sprites()
	{
		internal::sprite_batch.reset();
	}

	bool sprite_batch_is_rendering()
	{
		return internal::sprite_batch_rendering;
	}

	void internal::set_sprite_batch_rendering(bool ongoing)
	{
		internal::sprite_batch_rendering = ongoing;
	}

	rendering::SpriteBatch& sprite_batch()
	{
		return *internal::sprite_batch;
	}

	void render_sprites()
	{
		internal::sprite_batch->render();
		internal::set_sprite_batch_rendering(false);
	}
}
