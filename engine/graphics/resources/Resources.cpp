#include "Resources.h"

#include "graphics/resources/Shaders.h"
#include "graphics/resources/Samplers.h"
#include "graphics/resources/Textures.h"

namespace oly::graphics::internal
{
	void load_resources()
	{
		samplers::internal::load();
		internal_shaders::load();
		textures::internal::load();
	}

	void unload_resources()
	{
		samplers::internal::unload();
		internal_shaders::unload();
		textures::internal::unload();
	}
}
