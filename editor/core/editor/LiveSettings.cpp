#include "LiveSettings.h"

#include "core/editor/Logger.h"
#include "core/editor/ProjectInfo.h"

#include "definitions/Keys.h"

namespace oly::editor
{
	ContentBrowserLiveSettingsDesc::ContentBrowserLiveSettingsDesc() :
		columns(20u, detail::Key::Columns)
	{
	}

	LiveSettingsDesc::LiveSettingsDesc() :
		content_browser(detail::Key::ContentBrowser)
	{
	}

	void LiveSettings::Load()
	{
		std::filesystem::path path = GetPath().get_absolute();
		toml::table table;
		if (std::filesystem::is_regular_file(path))
		{
			try
			{
				table = toml::parse_file(path.string());
			}
			catch (const toml::parse_error& e)
			{
				Logger::Instance().Log(LogLevel::Warning, "Cannot load editor live settings: " + std::string(e.what()));
			}
		}

		desc.Load(TOMLNode(table));
	}

	void LiveSettings::Dump()
	{
		toml::table table;
		desc.Dump(table);
		std::filesystem::path path = GetPath().get_absolute();
		std::filesystem::create_directories(path.parent_path());
		std::ofstream file(path);
		file << table;
	}

	detail::ResourcePath LiveSettings::GetPath() const
	{
		return ProjectInfo::Instance().EditorRoot() / "LiveSettings.toml";
	}
}
