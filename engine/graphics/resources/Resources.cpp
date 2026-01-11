#include "Resources.h"

#include "graphics/resources/Shaders.h"
#include "graphics/resources/Samplers.h"
#include "graphics/resources/Textures.h"

namespace oly::graphics::internal
{
	struct UnloadResources
	{
		void operator()() const
		{
			samplers::internal::unload();
			internal_shaders::unload();
			textures::internal::unload();
		}
	};

	void load_resources()
	{
		samplers::internal::load();
		internal_shaders::load();
		textures::internal::load();
		SingletonTickService<TickPhase::None, void, TerminatePhase::Resources, UnloadResources>::instance();
	}
}
