#include "IDocument.h"

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
