#pragma once

#include "panels/IPanel.h"

class AssetEditorPanel : public IPanel
{
public:
	const char* GetTitle() const override;
	void Draw() override;
};
