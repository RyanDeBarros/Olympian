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

	extern BindlessTextureRef mod2x2(glm::vec<4, unsigned char> c1, glm::vec<4, unsigned char> c2, glm::vec<4, unsigned char> c3, glm::vec<4, unsigned char> c4);
	extern BindlessTextureRef mod2x2(glm::vec4 c1, glm::vec4 c2, glm::vec4 c3, glm::vec4 c4);
}
