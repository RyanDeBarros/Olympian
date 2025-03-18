#pragma once

#include "core/Core.h"

namespace oly
{
	namespace shaders
	{
		extern rendering::ShaderRes sprite_batch();
		extern rendering::ShaderRes polygon_batch();
		extern rendering::ShaderRes ellipse_batch();
		extern void unload();
	}
}
