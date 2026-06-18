#pragma once

#include "panels/IPanel.h"

namespace oly::editor
{
	class ContentBrowserPanel : public IPanel
	{
	public:
		static ContentBrowserPanel& Instance();

		void Init() override;
		const char* GetTitle() const override;
		void Draw() override;
	};
}
