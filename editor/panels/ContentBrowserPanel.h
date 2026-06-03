#pragma once

#include "panels/IPanel.h"

namespace oly::editor
{
	class ContentBrowserPanel : public IPanel
	{
	public:
		const char* GetTitle() const override;
		void Draw() override;
	};
}
