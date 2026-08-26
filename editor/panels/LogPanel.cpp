#include "LogPanel.h"

#include "core/windows/MainWindow.h"
#include "core/Errors.h"
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
			if (auto _ = imtk::child("##LogBox", ImVec2(), ImGuiChildFlags_Borders))
			{
				for (const imtk::log_entry& entry : imtk::log_entries())
				{
					if (auto _ = imtk::style_color(ImGuiCol_Text, imtk::log_level_color(entry.level)))
						ImGui::TextUnformatted(imtk::log_level_prefix(entry.level));

					ImGui::SameLine();
					ImGui::TextUnformatted(entry.msg.c_str());
				}

				if (auto _ = imtk::context_menu::window("##LogContextMenu"))
				{
					if (ImGui::MenuItem("Clear log"))
						imtk::clear_log();
				}
			}
		}
	}
}
