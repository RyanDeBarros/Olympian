#pragma once

#include "graphics/Texture.h"

namespace oly::editor
{
	enum class Resource
	{
		CollapseAllIcon,
		FilterOffIcon,
		FilterOnIcon,
		MinusIcon,
		PlusIcon,
		PreviewIcon,
		RecenterIcon,
		RevertIcon,
		RefreshIcon,
	};

	struct ResourceLoader
	{
		static void LoadAll();

		static const Texture& GetTexture(Resource resource);
	};
}
