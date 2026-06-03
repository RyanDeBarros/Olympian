#pragma once

#include "graphics/Texture.h"

namespace oly::editor
{
	enum class Resource
	{
		FilterOffIcon,
		FilterOnIcon,
	};

	struct ResourceLoader
	{
		static void LoadAll();

		static const Texture& GetTexture(Resource resource);
	};
}
