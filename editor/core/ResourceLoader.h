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
		PauseIcon,
		PlayIcon,
		PlusIcon,
		PreviewIcon,
		RecenterIcon,
		RevertIcon,
		RefreshIcon,
		StopIcon,
	};

	struct ResourceLoader
	{
		static void LoadAll();

		static const Texture& GetTexture(Resource resource);
	};
}
