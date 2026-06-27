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

	void* IDocument::VisitPath(DataPath path, std::type_index type)
	{
		return GetDoubleDescriptor().VisitPath(path, type);
	}

	void IDocument::DrawFinalize()
	{
		if (GetDoubleDescriptor().DrawFinalize())
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

	void IDocument::QueryDirty()
	{
		_dirty = GetDoubleDescriptor().QueryDirty();
	}

	// TODO v9.1 support holding down ctrl+z or ctrl+shift+z to undo/redo multiple actions over time

	void IDocument::Undo()
	{
		ActiveDocument active(*this);
		_undo_history->Undo();
	}

	void IDocument::Redo()
	{
		ActiveDocument active(*this);
		_undo_history->Redo();
	}

	IDocument::PreDrawImpl::PreDrawImpl(IDocument& doc) :
		_doc(doc), _uh_scope(*doc._undo_history), _active_instance(doc)
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
