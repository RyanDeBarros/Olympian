#pragma once

namespace oly::editor
{
#define ICON_RESOURCE_GENERATOR(M) \
	M(CircleLeft) \
	M(CirclePlus) \
	M(CircleRight) \
	M(Close) \
	M(CollapseAll) \
	M(Controller) \
	M(File) \
	M(FilterOff) \
	M(FilterOn) \
	M(Folder) \
	M(FolderOpen) \
	M(Handle) \
	M(Minus) \
	M(Pause) \
	M(Play) \
	M(Plus) \
	M(Preview) \
	M(Recenter) \
	M(Revert) \
	M(Refresh) \
	M(Settings) \
	M(StarFilled) \
	M(StarOutline) \
	M(Stop)


#define ICON_RESOURCE_ENUM(Icon) Icon,

	enum class IconResource : int
	{
		ICON_RESOURCE_GENERATOR(ICON_RESOURCE_ENUM)
	};

	class Texture;

	struct ResourceLoader
	{
		static void LoadAll();

		static Texture GetTexture(IconResource resource);
	};
}
