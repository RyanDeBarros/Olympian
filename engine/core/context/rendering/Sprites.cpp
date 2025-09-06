#include "Sprites.h"

#include "core/context/rendering/Rendering.h"
#include "graphics/primitives/Sprites.h"

namespace oly::context
{
	namespace internal
	{
		std::unique_ptr<rendering::SpriteBatch> sprite_batch;
	}

	void internal::init_sprites(const TOMLNode& node)
	{
		if (auto toml_sprite_batch = node["sprite_batch"])
		{
			int initial_sprites = 0;
			reg::parse_int(toml_sprite_batch, "initial sprites", initial_sprites);
			int new_textures = 0;
			reg::parse_int(toml_sprite_batch, "new_textures", new_textures);
			int new_uvs = 0;
			reg::parse_int(toml_sprite_batch, "new uvs", new_uvs);
			int new_modulations = 0;
			reg::parse_int(toml_sprite_batch, "new_modulations", new_modulations);
			int num_anims = 0;
			reg::parse_int(toml_sprite_batch, "num anims", num_anims);

			rendering::SpriteBatch::Capacity capacity{ (GLuint)initial_sprites, (GLuint)new_textures, (GLuint)new_uvs, (GLuint)new_modulations, (GLuint)num_anims };
			internal::sprite_batch = std::make_unique<rendering::SpriteBatch>(capacity);
		}
	}

	void internal::terminate_sprites()
	{
		internal::sprite_batch.reset();
	}

	rendering::SpriteBatch& sprite_batch()
	{
		return *internal::sprite_batch;
	}

	void render_sprites()
	{
		internal::sprite_batch->render();
		internal::set_batch_rendering_tracker(InternalBatch::SPRITE, false);
	}
}
