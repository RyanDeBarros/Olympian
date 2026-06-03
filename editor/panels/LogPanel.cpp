#include "LogPanel.h"

#include "core/Logger.h"

namespace oly::editor
{
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
		ImGui::Begin(GetTitle());
		ImGui::BeginChild("##log-box", ImVec2(), ImGuiChildFlags_Borders);

		for (const LogEntry& entry : Logger::Instance().Lines())
		{
			ImGui::PushStyleColor(ImGuiCol_Text, LogLevelColor(entry.level));
			ImGui::TextUnformatted(LogLevelPrefix(entry.level));
			ImGui::PopStyleColor();
			ImGui::SameLine();
			ImGui::TextUnformatted(entry.msg.c_str());
		}

		ImGui::EndChild();
		ImGui::End();
	}
}
