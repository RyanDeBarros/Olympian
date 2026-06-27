#pragma once

namespace oly::editor
{
	class IDocument;

	struct ActiveDocument
	{
		ActiveDocument(IDocument& doc);
		ActiveDocument(const ActiveDocument&) = delete;
		ActiveDocument(ActiveDocument&&) noexcept;
		~ActiveDocument();
		ActiveDocument& operator=(ActiveDocument&&) = delete;

		static IDocument& Get();
	};
}
