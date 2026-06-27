#include "IDocument.h"

#include <imgui.h>

namespace oly::editor
{
	IDocument::IDocument(detail::ResourcePath&& oly_path)
		: _oly_path(std::move(oly_path))
	{
	}

	void IDocument::Init()
	{
		_undo_history.emplace();
		InitImpl();
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

	void IDocument::DrawFinalize()
	{
		if (DrawFinalizeImpl())
			MarkDirty();
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

	// TODO v9.1 in undo actions, also store whether the document was dirty before the action. If it was NOT dirty, in Backward(), do a query on the document to check if the inverse action truly restored to a clean state, in which case we can do a QoL MarkClean().
	// TODO v9.1 support holding down ctrl+z or ctrl+shift+z to undo/redo multiple actions over time

	void IDocument::Undo()
	{
		DataPathVisitor _data_path_visitor([this](DataPath path, std::type_index type) { return VisitPath(path, type); });
		_undo_history->Undo();
	}

	void IDocument::Redo()
	{
		DataPathVisitor _data_path_visitor([this](DataPath path, std::type_index type) { return VisitPath(path, type); });
		_undo_history->Redo();
	}

	IDocument::PreDrawImpl::PreDrawImpl(IDocument& doc) :
		_doc(doc), _uh_scope(*doc._undo_history)
	{
	}

	IDocument::PreDrawImpl::~PreDrawImpl()
	{
		if (gui::PropertyGrid::DirtyGrid())
			_doc.MarkDirty();
	}

	IDocument::PreDrawImpl IDocument::PreDraw()
	{
		return PreDrawImpl(*this);
	}
}
