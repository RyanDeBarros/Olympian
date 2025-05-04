#include "Resources.h"

#include "graphics/resources/Shaders.h"
#include "graphics/resources/Samplers.h"
#include "graphics/resources/Textures.h"

namespace oly
{
	void load_resources()
	{
		samplers::load();
		shaders::load();
		textures::load();
	}

	void unload_resources()
	{
		samplers::unload();
		shaders::unload();
		textures::unload();
	}
}
