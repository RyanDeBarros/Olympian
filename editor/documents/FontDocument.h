#pragma once

#include "documents/IDocument.h"

namespace oly::editor
{
	class FontDocument : public IDocument
	{
	public:
		using IDocument::IDocument;

		void Init() override;
		void Draw() override;
		void Load() override;
		void Dump() override;

		detail::ResourcePath GetSourcePath() const;
	};
}
