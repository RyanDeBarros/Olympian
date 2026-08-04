#include "LogPanel.h"

#include "core/windows/MainWindow.h"
#include "core/Errors.h"
#include "core/editor/Logger.h"
#include "panels/PanelManager.h"

#include <imtk.hpp>

namespace oly::editor
{
	LogPanel& LogPanel::Instance()
	{
		if (auto panel = MainWindow::Instance().GetPanelManager().Get<LogPanel>())
			return *panel;
		else
			BreakoutError::Throw("No instance of LogPanel");
	}

	void LogPanel::InitImpl()
	{
		// nop
	}

	const char* LogPanel::GetTitle() const
	{
		return "Log";
	}

	void LogPanel::Draw()
	{
		auto window = DrawDockedWindow();
		if (window.IsVisible())
		{
			if (ImGui::BeginChild("##LogBox", ImVec2(), ImGuiChildFlags_Borders))
			{
				for (const LogEntry& entry : Logger::Instance().Lines())
				{
					if (auto _ = imtk::style_color(ImGuiCol_Text, LogLevelColor(entry.level)))
						ImGui::TextUnformatted(LogLevelPrefix(entry.level));

					ImGui::SameLine();
					ImGui::TextUnformatted(entry.msg.c_str());
				}

				if (ImGui::BeginPopupContextWindow("##LogContextMenu"))
				{
					if (ImGui::MenuItem("Clear log"))
						Logger::Instance().ClearLog();

					ImGui::EndPopup();
				}
			}

			ImGui::EndChild();
		}
	}
}
