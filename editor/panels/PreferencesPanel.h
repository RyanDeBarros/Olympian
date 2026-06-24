#pragma once

#include "panels/IPanel.h"
#include "documents/PreferencesDocument.h"

namespace oly::editor
{
	class PreferencesPanel : public IPanel
	{
		PreferencesDocument _doc;
		bool _unsaved_changes_modal = false;
		
	public:
		static PreferencesPanel& Instance();

		void Init() override;
		const char* GetTitle() const override;
		void Draw() override;

		const PreferencesDesc& GetActiveDesc() const;
		FunctionalEvent<>& OnActiveDescChanged();

	private:
		bool DrawUnsavedChangesModal();
	};
}
