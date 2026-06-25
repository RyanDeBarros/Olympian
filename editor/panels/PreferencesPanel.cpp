#include "PreferencesPanel.h"

#include "core/Errors.h"
#include "core/editor/Editor.h"
#include "core/windows/MainWindow.h"
#include "panels/PanelManager.h"

#include "definitions/Keys.h"

#include <imgui.h>

namespace oly::editor
{
	// TODO DEBT use kPascalCase notation for constants instead of all caps
	static constexpr const char* kWindowUnsavedChangesPopup = "Unsaved Changes##Window";
	static constexpr const char* kShutdownUnsavedChangesPopup = "Unsaved Changes##App";

	PreferencesPanel& PreferencesPanel::Instance()
	{
		if (auto panel = MainWindow::Instance().GetPanelManager().Get<PreferencesPanel>())
			return *panel;
		else
			BreakoutError::Throw("No instance of PreferencesPanel");
	}

	void PreferencesPanel::Init()
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

			_window_unsaved_changes_modal = true;
			ImGui::OpenPopup(kWindowUnsavedChangesPopup);
		}

		if (window.IsVisible())
		{
			if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_S, ImGuiInputFlags_RouteGlobal))
				_doc.Dump();

			_doc.DrawMenuBar();
			_doc.Draw();
		}

		if (_window_unsaved_changes_modal)
		{
			if (DrawUnsavedChangesModal(_window_unsaved_changes_modal, kWindowUnsavedChangesPopup))
				Close();
		}

		if (_shutdown_unsaved_changes_modal)
		{
			if (_open_shutdown_modal)
			{
				_open_shutdown_modal = false;
				ImGui::OpenPopup(kShutdownUnsavedChangesPopup);
			}

			if (DrawUnsavedChangesModal(_shutdown_unsaved_changes_modal, kShutdownUnsavedChangesPopup))
			{
				Close();
				Editor::Instance().RequestShutdown();
			}
		}
	}

	const PreferencesDesc& PreferencesPanel::GetActiveDesc() const
	{
		return _doc.GetActiveDesc();
	}

	FunctionalEvent<>& PreferencesPanel::OnActiveDescChanged()
	{
		return _doc.OnActiveDescChanged;
	}

	bool PreferencesPanel::DrawUnsavedChangesModal(bool& unsaved_changes_modal, const char* popup)
	{
		bool close_window = false;

		if (ImGui::BeginPopupModal(popup, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Editor preferences");

			if (ImGui::Button("Save Changes"))
			{
				_doc.Dump();
				ImGui::CloseCurrentPopup();
				unsaved_changes_modal = false;
				close_window = true;
			}

			ImGui::SameLine();
			if (ImGui::Button("Discard Changes"))
			{
				_doc.Load();
				ImGui::CloseCurrentPopup();
				unsaved_changes_modal = false;
				close_window = true;
			}

			ImGui::SameLine();
			if (ImGui::Button("Cancel Close"))
			{
				ImGui::CloseCurrentPopup();
				unsaved_changes_modal = false;
			}

			ImGui::EndPopup();
		}

		return close_window;
	}

	bool PreferencesPanel::RequestShutdown()
	{
		if (_doc.IsDirty())
		{
			Open();
			GainFocus();
			_shutdown_unsaved_changes_modal = true;
			_open_shutdown_modal = true;
			return false;
		}
		else
			return true;
	}
}
