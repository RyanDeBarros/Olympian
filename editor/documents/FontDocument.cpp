#include "FontDocument.h"

namespace oly::editor
{
	void FontDocument::Init()
	{
		// TODO v7
	}

	void FontDocument::Draw()
	{
		// TODO v7
	}

	void FontDocument::Load()
	{
		// TODO v7
	}

	void FontDocument::Dump()
	{
		// TODO v7
		MarkClean();
	}

	detail::ResourcePath FontDocument::GetSourcePath() const
	{
		return _oly_path.get_source_path();
	}
}
