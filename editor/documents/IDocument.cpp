#include "IDocument.h"

#include "gui/PropertyGrid.h"

namespace oly::editor
{
	IDocument::IDocument(detail::ResourcePath&& oly_path)
		: _oly_path(std::move(oly_path))
	{
	}

	void IDocument::DrawMenuBar()
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Save Changes", "Ctrl+S"))
					Dump();

				if (ImGui::MenuItem("Discard Changes"))
					Load();

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}
	}

	const detail::ResourcePath& IDocument::GetOlyPath() const
	{
		return _oly_path;
	}

	std::string IDocument::TabName() const
	{
		return _oly_path.tabname();
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

	IDocument::GridChecker::GridChecker(IDocument& doc)
		: _doc(doc)
	{
		gui::PropertyGrid::Clear();
	}

	IDocument::GridChecker::~GridChecker()
	{
		if (gui::PropertyGrid::DirtyGrid())
			_doc.MarkDirty();
	}

	IDocument::GridChecker IDocument::Grid()
	{
		return GridChecker(*this);
	}
}
