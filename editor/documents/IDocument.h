#pragma once

#include "assets/ResourcePath.h"

#include <imtk.hpp>

#include <imp/undo_history.hpp>

namespace oly::editor
{
    struct imtk::desc::idoubler;

    class IDocument
        : public imtk::tick_processor
        , public imtk::data_accessor
    {
    protected:
        detail::ResourcePath oly_path_;

    private:
        imp::checkpoint_undo_history undo_history_;
        imp::event_listener preferences_listener_;
        imp::event_listener uh_listener_;

        bool dirty_ = false;
        bool initialized_ = false;

    public:
        IDocument(detail::ResourcePath oly_path);
        virtual ~IDocument() = default;

    public:
        virtual void InitImpl() = 0;
        virtual void Draw() = 0;
        virtual void DrawMenuBar();

        virtual void LoadImpl() = 0;
        virtual void DumpImpl() = 0;
        virtual void ResetAssetImpl() = 0;

        virtual const imtk::desc::idoubler& GetDoubleDescriptor() const = 0;
        virtual imtk::desc::idoubler& GetDoubleDescriptor() = 0;

        void* resolve(imtk::datapath_view path, imp::type_erasure type) override;
        void describe(std::ostream& os, imtk::datapath_view path) const override;
        void on_last_process_frame() override;

    public:
        void Init();
        void LoadAsset();
        void DumpAsset();
        void ResetAsset();
        bool Exists();

        const detail::ResourcePath& GetOlyPath() const;
        void Rename(const detail::ResourcePath& new_path);
        virtual std::string TabName() const;

        void MarkDirty();
        void MarkClean();
        bool IsDirty() const;
        void query_dirty();

        void Undo();
        void Redo();

    private:
        class PreDrawImpl
        {
            IDocument& _doc;
            imp::active_undo_history _uh;
            imtk::active_data_accessor _active_instance;

        public:
            imtk::prop::grid grid;

            PreDrawImpl(IDocument& doc);
            PreDrawImpl(const PreDrawImpl&) = delete;
            PreDrawImpl(PreDrawImpl&&) = delete;
            ~PreDrawImpl();
        };

    protected:
        PreDrawImpl PreDraw();
    };
}
