#include "PreferencesPanel.h"

#include "core/editor/Editor.h"
#include "core/windows/MainWindow.h"
#include "panels/PanelManager.h"

#include "definitions/Keys.h"

#include <imgui.h>

namespace oly::editor
{
	PreferencesPanel& PreferencesPanel::Instance()
	{
		if (auto panel = MainWindow::Instance().GetPanelManager().Get<PreferencesPanel>())
			return *panel;
		else
			imtk::breakout_error::throw_("No instance of PreferencesPanel");
	}

	PreferencesPanel::PreferencesPanel()
		: _window_unsaved_changes_modal("Window", { "Editor preferences" })
		, _shutdown_unsaved_changes_modal("Shutdown", { "Editor preferences" })
	{
	}

	void PreferencesPanel::InitImpl()
	{
		_doc.Init();
	}

	const char* PreferencesPanel::GetTitle() const
	{
		return "Preferences";
	}

	void PreferencesPanel::Draw()
	{
		ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar;
		if (_doc.IsDirty())
			flags |= ImGuiWindowFlags_UnsavedDocument;

		auto window = DrawDockedWindow(flags);
		if (window.RequestsClose() && _doc.IsDirty())
		{
			Open();
			ImGui::SetWindowFocus();

			_window_unsaved_changes_modal.pop.open();
		}

		if (window.IsVisible())
		{
			if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_S, ImGuiInputFlags_RouteGlobal))
				_doc.DumpAsset();

			if (ImGui::Shortcut(ImGuiKey_Z | ImGuiMod_Ctrl, ImGuiInputFlags_RouteGlobal))
				_doc.Undo();

			if (ImGui::Shortcut(ImGuiKey_Z | ImGuiMod_Ctrl | ImGuiMod_Shift, ImGuiInputFlags_RouteGlobal))
				_doc.Redo();

			_doc.DrawMenuBar();
			_doc.Draw();
		}

		if (DrawUnsavedChangesModal(_window_unsaved_changes_modal))
			Close();

		if (DrawUnsavedChangesModal(_shutdown_unsaved_changes_modal))
		{
			Close();
			Editor::RequestShutdown();
		}
	}

	bool PreferencesPanel::DrawUnsavedChangesModal(imtk::unsaved_changes_modal& modal)
	{
		auto result = modal.draw();

		if (result == imtk::unsaved_changes_modal::result::save_changes)
			_doc.DumpAsset();

		if (result == imtk::unsaved_changes_modal::result::discard_changes)
			_doc.LoadAsset();

		return modal.closing(result);
	}

	bool PreferencesPanel::RequestShutdown()
	{
		if (_doc.IsDirty())
		{
			Open();
			GainFocus();
			_shutdown_unsaved_changes_modal.pop.open();
			return false;
		}
		else
			return true;
	}
}
