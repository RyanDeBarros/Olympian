#include "IDocument.h"

#include "core/editor/Editor.h"

#include "desc/PreferencesDesc.h"

#include <imgui.h>

namespace oly::editor
{
    IDocument::IDocument(detail::ResourcePath oly_path)
        : imtk::tick_processor(imtk::tick_process_phase::query_dirty)
        , oly_path_(std::move(oly_path))
        , undo_history_(
            Editor::GetPreferences().edit->undo_history->CountLimit(),
            Editor::GetPreferences().edit->undo_history->SizeLimit())
    {
        preferences_listener_ = Editor::OnPreferencesChanged().subscribe(
            [this]() {
                undo_history_.set_limits(
                    Editor::GetPreferences().edit->undo_history->CountLimit(),
                    Editor::GetPreferences().edit->undo_history->SizeLimit());
            });

        uh_listener_ = undo_history_.on_potential_clean.subscribe([this]() { query_dirty(); });
    }

    void* IDocument::resolve(imtk::datapath_view path, imp::type_erasure type)
    {
        return GetDoubleDescriptor().resolve(path, type);
    }

    void IDocument::describe(std::ostream& os, imtk::datapath_view path) const
    {
        GetDoubleDescriptor().describe(os, path);
    }

    void IDocument::on_last_process_frame()
    {
        query_dirty();
    }

    void IDocument::Init()
    {
        InitImpl();
        initialized_ = true;
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
        auto original = GetDoubleDescriptor().copy_scratch();

        ResetAssetImpl();
        query_dirty();

        std::unique_ptr<imp::undo_action> action;
        if (GetDoubleDescriptor().scratch_undo_action_query(std::move(original), action))
        {
            if (action)
                undo_history_.push(std::move(action));
            else
                undo_history_.clear();
        }
    }

    void IDocument::LoadAsset()
    {
        auto original = GetDoubleDescriptor().copy_scratch();

        LoadImpl();

        if (initialized_)
        {
            if (auto action = GetDoubleDescriptor().scratch_undo_action(std::move(original)))
                undo_history_.push(std::move(action));
            else
                undo_history_.clear();
        }
    }

    void IDocument::DumpAsset()
    {
        imtk::signal_publish_editing_sessions();
        DumpImpl();
    }

    bool IDocument::Exists()
    {
        return oly_path_.exists();
    }

    const detail::ResourcePath& IDocument::GetOlyPath() const
    {
        return oly_path_;
    }

    void IDocument::Rename(const detail::ResourcePath& new_path)
    {
        oly_path_ = new_path;
    }

    std::string IDocument::TabName() const
    {
        return oly_path_.tabname();
    }

    void IDocument::MarkDirty()
    {
        dirty_ = true;
    }

    void IDocument::MarkClean()
    {
        dirty_ = false;
        undo_history_.mark_clean();
    }

    bool IDocument::IsDirty() const
    {
        return dirty_;
    }

    void IDocument::query_dirty()
    {
        dirty_ = GetDoubleDescriptor().query_dirty();
    }

    void IDocument::Undo()
    {
        imtk::active_data_accessor active(*this);
        undo_history_.undo();
    }

    void IDocument::Redo()
    {
        imtk::active_data_accessor active(*this);
        undo_history_.redo();
    }

    IDocument::PreDrawImpl::PreDrawImpl(IDocument& doc)
        : _doc(doc)
        , _uh(doc.undo_history_)
        , _active_instance(doc)
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
