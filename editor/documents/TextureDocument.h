#pragma once

#include "documents/IDocument.h"

namespace oly::editor
{
	class TextureDocument : public IDocument
	{
		std::string GetTitle() const override;
		void Draw() override;
	};
}
