#include "ActiveDocument.h"

#include "documents/IDocument.h"
#include "core/Errors.h"

namespace oly::editor
{
	ActiveDocument* ACTIVE_INSTANCE = nullptr;
	IDocument* ACTIVE_DOCUMENT = nullptr;

	ActiveDocument::ActiveDocument(IDocument& doc)
	{
		if (ACTIVE_INSTANCE)
			BreakoutError::Throw("ActiveDocument::ActiveDocument(): active document already exists");

		ACTIVE_INSTANCE = this;
		ACTIVE_DOCUMENT = &doc;
	}

	ActiveDocument::ActiveDocument(ActiveDocument&& o) noexcept
	{
		if (ACTIVE_INSTANCE == &o)
			ACTIVE_INSTANCE = this;
	}

	ActiveDocument::~ActiveDocument()
	{
		if (ACTIVE_INSTANCE == this)
		{
			ACTIVE_INSTANCE = nullptr;
			ACTIVE_DOCUMENT = nullptr;
		}
	}

	IDocument& ActiveDocument::Get()
	{
		if (ACTIVE_DOCUMENT)
			return *ACTIVE_DOCUMENT;
		else
			BreakoutError::Throw("ActiveDocument::Get(): no active document");
	}
}
