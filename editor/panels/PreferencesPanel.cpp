#include "PreferencesPanel.h"

#include "core/Errors.h"
#include "core/windows/MainWindow.h"
#include "panels/PanelManager.h"

#include "definitions/Keys.h"

#include <imgui.h>

namespace oly::editor
{
	// TODO DEBT use kPascalCase notation for constants instead of all caps
	static constexpr const char* kUnsavedChangesPopup = "Unsaved Changes";

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

	// TODO v9.1 in AssetEditorPanel and PreferencesPanel, handle application on-close event for unsaved changes just like with tabs/windows close events

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

			_unsaved_changes_modal = true;
			ImGui::OpenPopup(kUnsavedChangesPopup);
		}

		if (window.IsVisible())
		{
			if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_S, ImGuiInputFlags_RouteGlobal))
				_doc.Dump();

			_doc.DrawMenuBar();
			_doc.Draw();
		}

		if (_unsaved_changes_modal)
		{
			if (DrawUnsavedChangesModal())
				Close();
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

	bool PreferencesPanel::DrawUnsavedChangesModal()
	{
		bool close_window = false;

		if (ImGui::BeginPopupModal(kUnsavedChangesPopup, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Editor preferences");

			if (ImGui::Button("Save Changes"))
			{
				_doc.Dump();
				ImGui::CloseCurrentPopup();
				_unsaved_changes_modal = false;
				close_window = true;
			}

			ImGui::SameLine();
			if (ImGui::Button("Discard Changes"))
			{
				_doc.Load();
				ImGui::CloseCurrentPopup();
				_unsaved_changes_modal = false;
				close_window = true;
			}

			ImGui::SameLine();
			if (ImGui::Button("Cancel Close"))
			{
				ImGui::CloseCurrentPopup();
				_unsaved_changes_modal = false;
			}

			ImGui::EndPopup();
		}

		return close_window;
	}
}
