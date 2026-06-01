#pragma once

#include "panels/IPanel.h"

namespace oly::editor
{
	class IDocument;

	class AssetEditorPanel : public IPanel
	{
		IDocument* _focused_tab = nullptr;

	public:
		static AssetEditorPanel& Instance();

		const char* GetTitle() const override;
		void Draw() override;

		void FocusTab(IDocument* doc);
		bool IsFocused(IDocument* doc) const;

		void SaveFocusedTab() const;
		void SaveAllTabs() const;
	};
}
