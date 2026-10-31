#pragma once

#include "desc/FontDesc.h"

#include "documents/IDocument.h"

#include "assets/MetaSplitter.h"

#include <imp/counter.hpp>
#include <imp/equal.hpp>
#include <imp/parser.hpp>

namespace oly::editor
{
    class FontDocument
        : public IDocument
    {
        imtk::desc::doubler<FullFontDesc> desc_;
        detail::MetaMap meta_;
        imtk::w::owned_list_indexer atlas_slots_;
        imtk::w::simple_widget<std::string> display_text_;
        imtk::font_instance preview_font_;
        imp::counter<
            std::array<std::string, 2>,
            imp::stl_hash<imp::cdpt_hash>,
            imp::stl_equal<imp::cdpt_equal>
        > glyph_counter_;

    public:
        FontDocument(detail::ResourcePath oly_path);

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

        detail::ResourcePath GetSourcePath() const;

    private:
        void DrawFontFace();
        void DrawFontAtlases();
        void DrawAtlasPreview();

        void Draw(FontFaceDesc& desc);
        void Draw(FontAtlasDesc& desc);

        void Load(imtk::toml_node node, FullFontDesc& desc);
        void Load(imtk::toml_node node, FontFaceDesc& desc);
        void Load(imtk::toml_node node, KerningDesc& desc);
        void Load(imtk::toml_node node, FontAtlasDesc& desc);

        void Dump(toml::table& table, FullFontDesc& desc);
        void Dump(toml::table& table, FontFaceDesc& desc);
        void Dump(toml::table& table, KerningDesc& desc);
        void Dump(toml::table& table, FontAtlasDesc& desc);
    };
}
