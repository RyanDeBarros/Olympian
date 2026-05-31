#pragma once

#include "IDocument.h"

class SpriteDocument : public IDocument
{
	std::string GetTitle() const override;
	void Draw() override;
};
