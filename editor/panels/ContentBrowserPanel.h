#pragma once

#include "panels/IPanel.h"

class ContentBrowserPanel : public IPanel
{
public:
	const char* GetTitle() const override;
	void Draw() override;
};
