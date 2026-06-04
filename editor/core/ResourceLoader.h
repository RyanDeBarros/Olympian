#pragma once

#include "graphics/Texture.h"

namespace oly::editor
{
#define ICON_RESOURCE_GENERATOR(M) \
	M(CollapseAllIcon) \
	M(FilterOffIcon) \
	M(FilterOnIcon) \
	M(MinusIcon) \
	M(PauseIcon) \
	M(PlayIcon) \
	M(PlusIcon) \
	M(PreviewIcon) \
	M(RecenterIcon) \
	M(RevertIcon) \
	M(RefreshIcon) \
	M(StopIcon)


#define ICON_RESOURCE_ENUM(Icon) Icon,

	enum class Resource
	{
		ICON_RESOURCE_GENERATOR(ICON_RESOURCE_ENUM)
	};

	struct ResourceLoader
	{
		static void LoadAll();

		static const Texture& GetTexture(Resource resource);
	};
}
