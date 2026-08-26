#include "ResourceLoader.h"

namespace oly::editor
{
	static std::unordered_map<IconResource, imtk::res::icon_id> icon_conversion;

	void LoadAllIcons()
	{
#define LOAD_ICON(Icon) icon_conversion[IconResource::Icon] = imtk::res::load_icon("res/icons/" #Icon ".png");
		ICON_RESOURCE_GENERATOR(LOAD_ICON)
#undef LOAD_ICON
	}

	imtk::res::icon_id Icon(IconResource resource)
	{
		return icon_conversion[resource];
	}

	imtk::texture GetIconTexture(IconResource resource)
	{
		if (imtk::res::icon_id id = Icon(resource))
			return imtk::res::icon_texture(id);
		else
		{
			switch (resource)
			{
#define THROW_MISSING_ICON(Icon) case IconResource::Icon: imtk::breakout_error::throw_("Texture not available for resource: \"" "res/icons/" #Icon ".png\"");
				ICON_RESOURCE_GENERATOR(THROW_MISSING_ICON)
#undef THROW_MISSING_ICON
			default:
				imtk::breakout_error::throw_("Texture not available for unknown resource: " + std::to_string(static_cast<int>(resource)));
			}
		}
	}
}
