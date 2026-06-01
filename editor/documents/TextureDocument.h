#pragma once

#include "documents/IDocument.h"

namespace oly::editor
{
	class TextureDocument : public IDocument
	{
	public:
		using IDocument::IDocument;

		void Draw() override;
	};
}
