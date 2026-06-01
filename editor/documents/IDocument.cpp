#include "IDocument.h"

namespace oly::editor
{
	IDocument::IDocument(detail::ResourcePath&& oly_path)
		: _oly_path(std::move(oly_path))
	{
	}

	const detail::ResourcePath& IDocument::GetOlyPath() const
	{
		return _oly_path;
	}

	void IDocument::MarkDirty()
	{
		_dirty = true;
	}

	void IDocument::MarkClean()
	{
		_dirty = false;
	}

	bool IDocument::IsDirty() const
	{
		return _dirty;
	}
}
