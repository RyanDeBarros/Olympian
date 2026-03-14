#include "Sprites.h"

#include "core/context/rendering/Rendering.h"

namespace oly::context
{
	namespace internal
	{
		std::unique_ptr<rendering::SpriteBatch> sprite_batch;
		bool sprite_batch_rendering = false;
	}

	struct SpriteBatchOnTerminate
	{
		void operator()() const
		{
			internal::sprite_batch.reset();
		}
	};

	void internal::init_sprites(TOMLNode node)
	{
		internal::sprite_batch = std::make_unique<rendering::SpriteBatch>();
		SingletonTickService<TickPhase::None, void, TerminatePhase::Graphics, SpriteBatchOnTerminate>::instance();
	}

	rendering::SpriteBatch& sprite_batch()
	{
		return *internal::sprite_batch;
	}

	void render_sprites()
	{
		(*internal::sprite_batch)->render();
	}
}
