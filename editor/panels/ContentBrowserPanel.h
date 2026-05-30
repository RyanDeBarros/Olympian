#pragma once

#include "IPanel.h"

class ContentBrowserPanel : public IPanel
{
public:
	const char* GetTitle() const override;
	void Draw() override;
};
