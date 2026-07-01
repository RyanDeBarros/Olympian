#include "ResourceLoader.h"

#include "core/Errors.h"
#include "gui/graphics/Texture.h"

#include <memory>
#include <string>

namespace oly::editor
{
#define TEXTURE_DECL(Icon) static Texture Icon##Icon;
	ICON_RESOURCE_GENERATOR(TEXTURE_DECL);
#undef TEXTURE_DECL

	void ResourceLoader::LoadAll()
	{
#define RES_FOLDER "res/"
#define ICONS_FOLDER RES_FOLDER "icons/"

#define LOAD_ICON(Icon) Icon##Icon = { Texture::LoadGeneric(ICONS_FOLDER #Icon ".png") };
		ICON_RESOURCE_GENERATOR(LOAD_ICON)
#undef LOAD_ICON

#undef ICONS_FOLDER
#undef RES_FOLDER
	}

	const Texture& ResourceLoader::GetTexture(IconResource resource)
	{
		switch (resource)
		{
#define SWITCH_ICON(Icon) case IconResource::Icon: return Icon##Icon;
			ICON_RESOURCE_GENERATOR(SWITCH_ICON)
#undef SWITCH_ICON
		}

		BreakoutError::Throw(("Texture not available for resource: " + std::to_string(static_cast<int>(resource))).c_str());
	}
}
