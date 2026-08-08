#include "IDocument.h"

#include "desc/DoubleDescriptor.h"

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
		_initialized = true;
	}

	void IDocument::DrawMenuBar()
	{
		if (auto _ = imtk::menu_bar())
		{
			if (auto _ = imtk::menu("File"))
			{
				if (ImGui::MenuItem("Save Changes", "Ctrl+S"))
					DumpAsset();

				if (ImGui::MenuItem("Discard Changes"))
					LoadAsset();

				if (ImGui::MenuItem("Reset Asset"))
					ResetAsset();
			}
		}
	}

	void IDocument::ResetAsset()
	{
		auto original = GetDoubleDescriptor().CopyScratch();

		ResetAssetImpl();
		QueryDirty();

		std::unique_ptr<UndoAction> action;
		if (GetDoubleDescriptor().ScratchUndoActionQuery(std::move(original), action))
		{
			if (action)
				_undo_history->Push(std::move(action));
			else
				_undo_history->Clear();
		}
	}

	void IDocument::LoadAsset()
	{
		auto original = GetDoubleDescriptor().CopyScratch();

		LoadImpl();

		if (_initialized)
		{
			if (auto action = GetDoubleDescriptor().ScratchUndoAction(std::move(original)))
				_undo_history->Push(std::move(action));
			else
				_undo_history->Clear();
		}
	}

	void IDocument::DumpAsset()
	{
		DumpImpl();
	}

	bool IDocument::Exists()
	{
		return _oly_path.exists();
	}

	void* IDocument::PathGet(DataPath path, std::type_index type)
	{
		return GetDoubleDescriptor().PathGet(path, type);
	}

	void IDocument::PrintPath(std::ostream& os, DataPath path) const
	{
		GetDoubleDescriptor().PrintPath(os, path);
	}
	
	std::string IDocument::PathString(DataPath path) const
	{
		std::stringstream ss;
		PrintPath(ss, path);
		return ss.str();
	}

	// TODO v9.3 put in on_last_process_frame() instead. Use priority system to enforce order (submit_edit -> check_undo -> query_dirty) instead of calling on_last_process_frame() - make protected
	void IDocument::DrawFinalize()
	{
		QueryDirty();
	}

	const detail::ResourcePath& IDocument::GetOlyPath() const
	{
		return _oly_path;
	}

	void IDocument::Rename(const detail::ResourcePath& new_path)
	{
		_oly_path = new_path;
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
		_undo_history->MarkClean();
	}

	bool IDocument::IsDirty() const
	{
		return _dirty;
	}

	void IDocument::QueryDirty()
	{
		_dirty = GetDoubleDescriptor().QueryDirty();
	}

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
