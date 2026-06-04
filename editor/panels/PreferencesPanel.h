#pragma once

#include "panels/IPanel.h"

namespace oly::editor
{
	class PreferencesPanel : public IPanel
	{
	public:
		static PreferencesPanel& Instance();

		void Init() override;
		const char* GetTitle() const override;
		void Draw() override;
	};
}
