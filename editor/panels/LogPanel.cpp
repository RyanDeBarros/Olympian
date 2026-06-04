#include "LogPanel.h"

#include "core/MainWindow.h"
#include "core/Errors.h"
#include "core/Logger.h"
#include "panels/PanelManager.h"

namespace oly::editor
{
	LogPanel& LogPanel::Instance()
	{
		if (auto panel = MainWindow::Instance().GetPanelManager().Get<LogPanel>())
			return *panel;
		else
			BreakoutError::Throw("No instance of LogPanel");
	}

	void LogPanel::Init()
	{
		// nop
	}

	const char* LogPanel::GetTitle() const
	{
		return "Log";
	}

	void LogPanel::Draw()
	{
		if (auto window = DrawDockedWindow(ImGuiWindowFlags_None))
		{
			if (ImGui::BeginChild("##LogBox", ImVec2(), ImGuiChildFlags_Borders))
			{
				for (const LogEntry& entry : Logger::Instance().Lines())
				{
					ImGui::PushStyleColor(ImGuiCol_Text, LogLevelColor(entry.level));
					ImGui::TextUnformatted(LogLevelPrefix(entry.level));
					ImGui::PopStyleColor();
					ImGui::SameLine();
					ImGui::TextUnformatted(entry.msg.c_str());
				}

			}

			ImGui::EndChild();
		}
	}
}
