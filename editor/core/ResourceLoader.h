#pragma once

#include "graphics/Texture.h"

namespace oly::editor
{
	enum class Resource
	{
		CollapseAllIcon,
		FilterOffIcon,
		FilterOnIcon,
		RecenterIcon,
		RevertIcon,
		RefreshIcon,
		PlusIcon,
		MinusIcon,
	};

	struct ResourceLoader
	{
		static void LoadAll();

		static const Texture& GetTexture(Resource resource);
	};
}
