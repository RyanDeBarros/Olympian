#pragma once

#include "panels/IPanel.h"
#include "documents/PreferencesDocument.h"

namespace oly::editor
{
	class PreferencesPanel : public IPanel
	{
		PreferencesDocument _doc;
		imtk::unsaved_changes_modal _window_unsaved_changes_modal;
		imtk::unsaved_changes_modal _shutdown_unsaved_changes_modal;
		
	public:
		static PreferencesPanel& Instance();

		PreferencesPanel();

		void InitImpl() override;
		const char* GetTitle() const override;
		void Draw() override;

	private:
		bool DrawUnsavedChangesModal(imtk::unsaved_changes_modal& modal);

	public:
		bool RequestShutdown();
	};
}
