#pragma once

#include "documents/IDocument.h"

class TextureDocument : public IDocument
{
	std::string GetTitle() const override;
	void Draw() override;
};
