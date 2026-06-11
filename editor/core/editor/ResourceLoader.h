#pragma once

namespace oly::editor
{
#define ICON_RESOURCE_GENERATOR(M) \
	M(CollapseAll) \
	M(Close) \
	M(FilterOff) \
	M(FilterOn) \
	M(Handle) \
	M(Minus) \
	M(Pause) \
	M(Play) \
	M(Plus) \
	M(Preview) \
	M(Recenter) \
	M(Revert) \
	M(Refresh) \
	M(Stop)


#define ICON_RESOURCE_ENUM(Icon) Icon,

	enum class IconResource
	{
		ICON_RESOURCE_GENERATOR(ICON_RESOURCE_ENUM)
	};

	class Texture;

	struct ResourceLoader
	{
		static void LoadAll();

		static const Texture& GetTexture(IconResource resource);
	};
}
