#pragma once

#include "panels/IPanel.h"

namespace oly::editor
{
	class IDocument;

	class AssetEditorPanel : public IPanel
	{
		IDocument* _selected_tab = nullptr;
		IDocument* _focused_tab = nullptr;

	public:
		static AssetEditorPanel& Instance();

		const char* GetTitle() const override;
		void Draw() override;

		void FocusTab(IDocument* doc);
		bool IsSelected(IDocument* doc) const;

		void SaveSelectedTab() const;
		void SaveAllTabs() const;
	};
}
