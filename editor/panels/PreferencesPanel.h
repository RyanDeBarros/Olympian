#pragma once

#include "panels/IPanel.h"
#include "documents/PreferencesDocument.h"

namespace oly::editor
{
	class PreferencesPanel : public IPanel
	{
		PreferencesDocument doc;
		
	public:
		static PreferencesPanel& Instance();

		void Init() override;
		const char* GetTitle() const override;
		void Draw() override;

		const PreferencesDesc& GetSavedDesc() const;
	};
}
