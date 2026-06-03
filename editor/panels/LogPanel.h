#pragma once

#include "panels/IPanel.h"

namespace oly::editor
{
	class LogPanel : public IPanel
	{
	public:
		void Init() override;
		const char* GetTitle() const override;
		void Draw() override;
	};
}
