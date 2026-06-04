#include "ResourceLoader.h"

#include "core/Errors.h"

#include <memory>
#include <string>

namespace oly::editor
{
	// TODO v8 simplify loading with macro: rename textures to PascalCase_icon to easily use generator macro

	static Texture collapse_all_icon;
	static Texture filter_off_icon;
	static Texture filter_on_icon;
	static Texture minus_icon;
	static Texture pause_icon;
	static Texture play_icon;
	static Texture plus_icon;
	static Texture preview_icon;
	static Texture recenter_icon;
	static Texture revert_icon;
	static Texture refresh_icon;
	static Texture stop_icon;

	void ResourceLoader::LoadAll()
	{
#define RES_FOLDER "res/"
#define ICONS_FOLDER RES_FOLDER "icons/"

		collapse_all_icon = { RasterTexture(ICONS_FOLDER "CollapseAll.png")};
		filter_off_icon = { RasterTexture(ICONS_FOLDER "FilterOff.png")};
		filter_on_icon = { RasterTexture(ICONS_FOLDER "FilterOn.png")};
		minus_icon = { RasterTexture(ICONS_FOLDER "Minus.png" ) };
		pause_icon = { RasterTexture(ICONS_FOLDER "Pause.png") };
		play_icon = { RasterTexture(ICONS_FOLDER "Play.png") };
		plus_icon = { RasterTexture(ICONS_FOLDER "Plus.png") };
		preview_icon = { RasterTexture(ICONS_FOLDER "Preview.png") };
		recenter_icon = { RasterTexture(ICONS_FOLDER "Recenter.png") };
		revert_icon = { RasterTexture(ICONS_FOLDER "Revert.png") };
		refresh_icon = { RasterTexture(ICONS_FOLDER "Refresh.png") };
		stop_icon = { RasterTexture(ICONS_FOLDER "Stop.png") };

#undef ICONS_FOLDER
#undef RES_FOLDER
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
		case Resource::MinusIcon:
			return minus_icon;
		case Resource::PauseIcon:
			return pause_icon;
		case Resource::PlayIcon:
			return play_icon;
		case Resource::PlusIcon:
			return plus_icon;
		case Resource::PreviewIcon:
			return preview_icon;
		case Resource::RecenterIcon:
			return recenter_icon;
		case Resource::RevertIcon:
			return revert_icon;
		case Resource::RefreshIcon:
			return refresh_icon;
		case Resource::StopIcon:
			return stop_icon;
		}

		BreakoutError::Throw(("Texture not available for resource: " + std::to_string(static_cast<int>(resource))).c_str());
	}
}
