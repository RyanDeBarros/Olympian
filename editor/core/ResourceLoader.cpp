#include "ResourceLoader.h"

#include "core/Errors.h"

#include <memory>
#include <string>

namespace oly::editor
{
#define TEXTURE_DECL(Icon) static Texture Icon;
	ICON_RESOURCE_GENERATOR(TEXTURE_DECL);
#undef TEXTURE_DECL

	void ResourceLoader::LoadAll()
	{
#define RES_FOLDER "res/"
#define ICONS_FOLDER RES_FOLDER "icons/"

#define LOAD_ICON(Icon) Icon = { RasterTexture(ICONS_FOLDER #Icon ".png") };
		ICON_RESOURCE_GENERATOR(LOAD_ICON)
#undef LOAD_ICON

#undef ICONS_FOLDER
#undef RES_FOLDER
	}

	const Texture& ResourceLoader::GetTexture(Resource resource)
	{
		switch (resource)
		{
#define SWITCH_ICON(Icon) case Resource::Icon: return Icon;
			ICON_RESOURCE_GENERATOR(SWITCH_ICON)
#undef SWITCH_ICON
		}

		BreakoutError::Throw(("Texture not available for resource: " + std::to_string(static_cast<int>(resource))).c_str());
	}
}
