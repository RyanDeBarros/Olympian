#include "ResourceLoader.h"

#include "core/Errors.h"

#include <memory>
#include <string>

namespace oly::editor
{
	static Texture collapse_all_icon;
	static Texture filter_off_icon;
	static Texture filter_on_icon;

	void ResourceLoader::LoadAll()
	{
#define RES_FOLDER "res/"
#define ICONS_FOLDER RES_FOLDER "icons/"

		collapse_all_icon = { RasterTexture(ICONS_FOLDER "CollapseAll.png")};
		filter_off_icon = { RasterTexture(ICONS_FOLDER "FilterOff.png")};
		filter_on_icon = { RasterTexture(ICONS_FOLDER "FilterOn.png")};

#undef RES_FOLDER
#undef ICONS_FOLDER
	}

	const Texture& ResourceLoader::GetTexture(Resource resource)
	{
		switch (resource)
		{
		case Resource::CollapseAllIcon:
			return collapse_all_icon;
		case Resource::FilterOffIcon:
			return filter_off_icon;
		case Resource::FilterOnIcon:
			return filter_on_icon;
		}

		BreakoutError::Throw(("Texture not available for resource: " + std::to_string(static_cast<int>(resource))).c_str());
	}
}
