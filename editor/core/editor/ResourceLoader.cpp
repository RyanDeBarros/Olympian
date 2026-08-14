#include "ResourceLoader.h"

#include "core/Errors.h"

namespace oly::editor
{
	static std::unordered_map<IconResource, imtk::res::icon_id> icon_conversion;

	void ResourceLoader::LoadAll()
	{
#define LOAD_ICON(Icon) icon_conversion[IconResource::Icon] = imtk::res::load_icon("res/icons/" #Icon ".png");
		ICON_RESOURCE_GENERATOR(LOAD_ICON)
#undef LOAD_ICON
	}

	imtk::texture ResourceLoader::GetTexture(IconResource resource)
	{
		if (imtk::res::icon_id id = icon_conversion[resource])
			return imtk::res::icon_texture(id);
		else
		{
			switch (resource)
			{
#define THROW_MISSING_ICON(Icon) case IconResource::Icon: BreakoutError::Throw("Texture not available for resource: \"" "res/icons/" #Icon ".png\"");
				ICON_RESOURCE_GENERATOR(THROW_MISSING_ICON)
#undef THROW_MISSING_ICON
			default:
				BreakoutError::Throw("Texture not available for unknown resource: " + std::to_string(static_cast<int>(resource)));
			}
		}
	}
}
