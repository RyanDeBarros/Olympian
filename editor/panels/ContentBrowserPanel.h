#pragma once

#include "panels/IPanel.h"

namespace oly::editor
{
	class ContentBrowserPanel : public IPanel
	{
	public:
		void Init() override;
		const char* GetTitle() const override;
		void Draw() override;
	};
}
