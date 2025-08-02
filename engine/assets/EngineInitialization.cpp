#include "EngineInitialization.h"

#include "core/base/Context.h"

namespace oly::assets::internal
{
	static void load_olympian(const Asset& asset)
	{
		// TODO v3

		for (const Asset::Line& line : asset.content)
		{
			std::vector<std::string_view> params = split(line.line, ':');
			if (params[0] == "!w")
			{
			}
			else if (params[0] == "!wh")
			{
			}
			else if (params[0] == "!g")
			{
			}
		}
	}

	void init_engine()
	{
		OlyScriptReadResult result = read_asset(context::resource_file("Olympian.oly"));
		
		if (result.assets.empty())
		{
			LOG << LOG.begin_temp(LOG.fatal) << "Olympian.oly does not contain any asset sections." << LOG.end_temp << LOG.nl;
			throw Error(ErrorCode::CONTEXT_INIT);
		}

		if (result.assets[0].header != "oly")
		{
			LOG << LOG.begin_temp(LOG.fatal) << "First asset section in Olympian.oly does not have \"oly\" header" << LOG.end_temp << LOG.nl;
			throw Error(ErrorCode::CONTEXT_INIT);
		}

		internal::load_olympian(result.assets[0]);

		if (result.assets.size() > 1)
			LOG << LOG.begin_temp(LOG.warning) << "Olympian.oly contains additional asset sections after \"oly\". Ignoring them..." << LOG.end_temp << LOG.nl;
	}
}
