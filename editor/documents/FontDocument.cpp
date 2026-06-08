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
		if (!GetSourcePath().is_resource())
		{
			Notification notif(LogLevel::Warning, "Asset is not located in resource folder");
			MainWindow::Instance().PushNotification(std::move(notif));
		}

		Load();
	}

	void FontDocument::Draw()
	{
		ImGui::PushID(this);

		// TODO v8

		ImGui::PopID();
	}

	void FontDocument::DrawMenuBar()
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Save Changes", "Ctrl+S"))
					Dump();

				if (ImGui::MenuItem("Discard Changes"))
					Load();

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}
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
