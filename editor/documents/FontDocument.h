#pragma once

#include "documents/IDocument.h"

namespace oly::editor
{
	class FontDocument : public IDocument
	{
	public:
		using IDocument::IDocument;

		void Draw() override;
	};
}
