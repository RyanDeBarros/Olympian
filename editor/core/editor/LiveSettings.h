#pragma once

#include "desc/SimpleField.h"

#include "assets/ResourcePath.h"

namespace oly::editor
{
	struct ContentBrowserLiveSettingsDesc
	{
#define CONTENT_BROWSER_LIVE_SETTINGS_GENERATOR(M) \
		M(rows)

		SimpleField<unsigned int> rows;

		LOAD_DUMP_SIMPLE_FIELDS_IMPL(CONTENT_BROWSER_LIVE_SETTINGS_GENERATOR);

#undef CONTENT_BROWSER_LIVE_SETTINGS_GENERATOR

		ContentBrowserLiveSettingsDesc();
	};

	struct LiveSettingsDesc
	{
#define LIVE_SETTINGS_GENERATOR(M) \
		M(content_browser)

		SimpleDesc<ContentBrowserLiveSettingsDesc> content_browser;

		LOAD_DUMP_SIMPLE_FIELDS_IMPL(LIVE_SETTINGS_GENERATOR);

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
