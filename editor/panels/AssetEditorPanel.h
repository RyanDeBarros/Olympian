#pragma once

#include "IPanel.h"

class AssetEditorPanel : public IPanel
{
public:
	const char* GetTitle() const override;
	void Draw() override;
};
