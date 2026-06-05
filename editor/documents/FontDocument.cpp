#include "FontDocument.h"

#include "core/MainWindow.h"
#include "core/Logger.h"

namespace oly::editor
{
	const char* FontDocument::GetVersion()
	{
		return "1.0";
	}

	void FontDocument::Init()
	{
		// TODO v8

		if (!GetSourcePath().is_resource())
		{
			Notification notif(LogLevel::Warning, "Asset is not located in resource folder");
			MainWindow::Instance().PushNotification(std::move(notif));
		}
	}

	void FontDocument::Draw()
	{
		// TODO v8
	}

	void FontDocument::DrawMenuBar()
	{
		// TODO v8
	}

	void FontDocument::Load()
	{
		// TODO v8
	}

	void FontDocument::Dump()
	{
		// TODO v8
		MarkClean();
	}

	detail::ResourcePath FontDocument::GetSourcePath() const
	{
		return _oly_path.get_source_path();
	}
}
