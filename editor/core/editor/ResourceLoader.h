#pragma once

namespace oly::editor
{
#define ICON_RESOURCE_GENERATOR(M) \
	M(ChevronDown) \
	M(ChevronRight) \
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
	M(FontFamily) \
	M(Font) \
	M(Handle) \
	M(Import) \
	M(Minus) \
	M(OpenInTreeView) \
	M(Pause) \
	M(Play) \
	M(Plus) \
	M(Preview) \
	M(Prune) \
	M(RasterFont) \
	M(Recenter) \
	M(Revert) \
	M(Refresh) \
	M(Settings) \
	M(StarFilled) \
	M(StarOutline) \
	M(Stop) \
	M(Texture) \
	M(Tileset)


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
