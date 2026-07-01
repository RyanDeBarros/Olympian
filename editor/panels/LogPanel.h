#pragma once

#include "panels/IPanel.h"

namespace oly::editor
{
	class LogPanel : public IPanel
	{
	public:
		static LogPanel& Instance();

		void InitImpl() override;
		const char* GetTitle() const override;
		void Draw() override;
	};
}
