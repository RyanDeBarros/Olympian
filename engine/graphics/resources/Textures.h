#pragma once

#include "graphics/backend/basic/Textures.h"

namespace oly::graphics::textures
{
	extern BindlessTextureRef white1x1;

	namespace internal
	{
		extern void load();
		extern void unload();
	}
}
