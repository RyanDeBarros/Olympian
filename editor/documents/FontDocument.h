#pragma once

#include "documents/IDocument.h"

namespace oly::editor
{
	class FontDocument : public IDocument
	{
	public:
		using IDocument::IDocument;

		static const char* GetVersion();

		void Init() override;
		void Draw() override;
		void DrawMenuBar() override;
		void Load() override;
		void Dump() override;

		detail::ResourcePath GetSourcePath() const;
	};
}
