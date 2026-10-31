#pragma once

#include "desc/RasterFontDesc.h"

#include "documents/IDocument.h"

#include "assets/MetaSplitter.h"

namespace oly::editor
{
    class RasterFontDocument
        : public IDocument
    {
        imtk::desc::doubler<RasterFontDesc> desc_;
        detail::MetaMap meta_;
        imtk::w::owned_list_indexer glyphs_;
        imp::counter<std::string> codepoint_counter_;

    public:
        RasterFontDocument(detail::ResourcePath oly_path);

    public:
        void InitImpl() override;
        void Draw() override;
        void LoadImpl() override;
        void DumpImpl() override;
        void ResetAssetImpl() override;
        const imtk::desc::idoubler& GetDoubleDescriptor() const override;
        imtk::desc::idoubler& GetDoubleDescriptor() override;

    public:
        static const char* GetVersion();

    private:
        void Draw(RasterFontDesc& desc);
        void Draw(GlyphDesc& desc);

        void Load(imtk::toml_node node, RasterFontDesc& desc);
        void Load(imtk::toml_node node, GlyphDesc& desc);

        void Dump(toml::table& table, RasterFontDesc& desc);
        void Dump(toml::table& table, GlyphDesc& desc);

        imtk::list_adapter ListAdapter();
        std::unique_ptr<imtk::ilist_op_adapter> ListOpAdapter();
    };
}
