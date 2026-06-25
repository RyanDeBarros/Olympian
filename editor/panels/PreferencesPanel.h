#pragma once

#include "panels/IPanel.h"
#include "documents/PreferencesDocument.h"

namespace oly::editor
{
	class PreferencesPanel : public IPanel
	{
		PreferencesDocument _doc;
		bool _window_unsaved_changes_modal = false;
		bool _shutdown_unsaved_changes_modal = false;
		bool _open_shutdown_modal = false;
		
	public:
		static PreferencesPanel& Instance();

		void Init() override;
		const char* GetTitle() const override;
		void Draw() override;

		const PreferencesDesc& GetActiveDesc() const;
		FunctionalEvent<>& OnActiveDescChanged();

	private:
		bool DrawUnsavedChangesModal(bool& unsaved_changes_modal, const char* popup);

	public:
		bool RequestShutdown();
	};
}
