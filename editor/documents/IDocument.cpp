#include "IDocument.h"

#include "core/editor/Editor.h"

#include "desc/DoubleDescriptor.h"
#include "desc/impl/PreferencesDesc.h"

#include <imgui.h>

namespace oly::editor
{
	IDocument::IDocument(detail::ResourcePath&& oly_path)
		: imtk::tick_processor(imtk::tick_process_phase::query_dirty), _oly_path(std::move(oly_path)),
		_undo_history(Editor::GetPreferences().edit->undo_history->CountLimit(), Editor::GetPreferences().edit->undo_history->SizeLimit())
	{
		_preferences_listener = Editor::OnPreferencesChanged().subscribe([this]() {
			_undo_history.set_limits(Editor::GetPreferences().edit->undo_history->CountLimit(), Editor::GetPreferences().edit->undo_history->SizeLimit());
		});

		_uh_listener = _undo_history.on_potential_clean.subscribe([this]() { query_dirty(); });
	}

	void IDocument::Init()
	{
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
		query_dirty();

		std::unique_ptr<imp::undo_action> action;
		if (GetDoubleDescriptor().ScratchUndoActionQuery(std::move(original), action))
		{
			if (action)
				_undo_history.push(std::move(action));
			else
				_undo_history.clear();
		}
	}

	void IDocument::LoadAsset()
	{
		auto original = GetDoubleDescriptor().CopyScratch();

		LoadImpl();

		if (_initialized)
		{
			if (auto action = GetDoubleDescriptor().ScratchUndoAction(std::move(original)))
				_undo_history.push(std::move(action));
			else
				_undo_history.clear();
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

	void* IDocument::resolve(imtk::datapath_view path, std::type_index type)
	{
		return GetDoubleDescriptor().resolve(path, type);
	}

	void IDocument::describe(std::ostream& os, imtk::datapath_view path) const
	{
		GetDoubleDescriptor().describe(os, path);
	}
	
	std::string IDocument::PathString(imtk::datapath_view path) const
	{
		std::stringstream ss;
		describe(ss, path);
		return ss.str();
	}

	void IDocument::on_last_process_frame()
	{
		query_dirty();
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
		_undo_history.mark_clean();
	}

	bool IDocument::IsDirty() const
	{
		return _dirty;
	}

	void IDocument::query_dirty()
	{
		_dirty = GetDoubleDescriptor().query_dirty();
	}

	void IDocument::Undo()
	{
		ActiveDocument active(*this);
		_undo_history.undo();
	}

	void IDocument::Redo()
	{
		ActiveDocument active(*this);
		_undo_history.redo();
	}

	IDocument::PreDrawImpl::PreDrawImpl(IDocument& doc) :
		_doc(doc), _uh(doc._undo_history), _active_instance(doc)
	{
	}

	IDocument::PreDrawImpl::~PreDrawImpl()
	{
		if (grid.dirty())
			_doc.MarkDirty();
	}

	IDocument::PreDrawImpl IDocument::PreDraw()
	{
		return PreDrawImpl(*this);
	}
}
