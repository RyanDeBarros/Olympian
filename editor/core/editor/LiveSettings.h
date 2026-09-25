#pragma once

#include "bindings/Serializer.h"

#include "assets/ResourcePath.h"

namespace oly::editor
{
	struct ContentBrowserLiveSettingsDesc
	{
#define CONTENT_BROWSER_LIVE_SETTINGS_GENERATOR(M) \
		M(columns) \
		M(favorites) \
		M(font_scale)

		imtk::field::simple<unsigned int> columns;
		imtk::field::simple<std::set<detail::ResourcePath>> favorites;
		imtk::field::simple<float> font_scale;

		IMTK_LOAD_DUMP_SIMPLE_FIELDS_IMPL(CONTENT_BROWSER_LIVE_SETTINGS_GENERATOR);

#undef CONTENT_BROWSER_LIVE_SETTINGS_GENERATOR

		ContentBrowserLiveSettingsDesc();
	};

	struct LiveSettingsDesc
	{
#define LIVE_SETTINGS_GENERATOR(M) \
		M(content_browser)

		imtk::desc::simple<ContentBrowserLiveSettingsDesc> content_browser;

		IMTK_LOAD_DUMP_SIMPLE_FIELDS_IMPL(LIVE_SETTINGS_GENERATOR);

#undef LIVE_SETTINGS_GENERATOR

		LiveSettingsDesc();
	};

	struct LiveSettings
	{
		LiveSettingsDesc desc;
		
		void Load();
		void Dump();

		detail::ResourcePath GetPath() const;
	};
}
