#include "FontFamilyDocument.h"

#include "assets/TranslateKey.h"
#include "definitions/Keys.h"

#include <imp/parser.hpp>

namespace oly::editor
{
    void FontFamilyDocument::InitImpl()
    {
        if (!GetOlyPath().is_resource())
            imtk::notify_warning("Asset is not located in resource folder");

        LoadAsset();
    }

    void FontFamilyDocument::Draw()
    {
        auto pre_draw = PreDraw();

        imtk::id_scope scope(this);
        if (auto form = imtk::prop::form())
        {
            Draw(desc_.scratch, "Regular", detail::FontStyleMode::Regular);
            Draw(desc_.scratch, "Bold", detail::FontStyleMode::Bold);
            Draw(desc_.scratch, "Italic", detail::FontStyleMode::Italic);
            Draw(desc_.scratch, "Bold-italic", detail::FontStyleMode::BoldItalic);
        }
    }

    void FontFamilyDocument::LoadImpl()
    {
        if (oly_path_.is_file())
        {
            meta_ = detail::MetaSplitter::decode_meta(oly_path_);

            toml::table table;
            std::string err = oly_path_.load_toml(table);
            if (err.empty())
                Load(imtk::toml_node(table), desc_.disk);
            else
                imtk::notify_error("cannot load font family - corrupted asset: " + oly_path_.string());

            MarkClean();
        }
        else
        {
            Load(imtk::toml_node(), desc_.disk);

            meta_ = {};
            meta_.map[detail::Key::Meta_Version] = GetVersion();
            meta_.map[detail::Key::Meta_Import] = "0";
            meta_.map[detail::Key::Meta_Type] = detail::encode_key(detail::Key::Meta_FontFamily);

            MarkDirty();
        }

        desc_.load_from_disk();
    }

    void FontFamilyDocument::DumpImpl()
    {
        toml::table table;
        Dump(table, desc_.scratch);
        oly_path_.dump_toml(table, meta_);
        desc_.write_to_disk();
        MarkClean();
    }

    void FontFamilyDocument::ResetAssetImpl()
    {
        Load(imtk::toml_node(), desc_.scratch);
    }

    const imtk::desc::idoubler& FontFamilyDocument::GetDoubleDescriptor() const
    {
        return desc_;
    }

    imtk::desc::idoubler& FontFamilyDocument::GetDoubleDescriptor()
    {
        return desc_;
    }

    const char* FontFamilyDocument::GetVersion()
    {
        return "1.0";
    }

    void FontFamilyDocument::Draw(FontFamilyDesc& desc, const char* subform_header, detail::FontStyleMode style)
    {
        if (auto subform = imtk::prop::subform(subform_header))
            Draw(desc.styles[style]);
    }

    void FontFamilyDocument::Draw(FontStyleDesc& desc)
    {
        IMTK_DRAW_FIELDS(STYLE_GENERATOR);
    }

    void FontFamilyDocument::Load(imtk::toml_node node, FontFamilyDesc& desc)
    {
        desc.styles.clear();

        if (auto table = desc.styles.subnode(node).as_table())
        {
            for (auto&& [key, subnode] : *table)
            {
                if (auto style = imp::stoi(key.str()))
                    Load(imtk::toml_node(subnode), desc.styles[static_cast<detail::FontStyleMode>(*style)]);
            }
        }
    }

    void FontFamilyDocument::Load(imtk::toml_node node, FontStyleDesc& desc)
    {
        IMTK_LOAD_FIELDS(STYLE_GENERATOR);
    }

    void FontFamilyDocument::Dump(toml::table& table, FontFamilyDesc& desc)
    {
        toml::table subtable;
        for (auto&& [style, subdesc] : desc.styles)
        {
            toml::table inner;
            Dump(inner, subdesc);
            subtable.insert_or_assign(std::to_string(style), std::move(inner));
        }
        desc.styles.dump_into(table, std::move(subtable));
    }

    void FontFamilyDocument::Dump(toml::table& table, FontStyleDesc& desc)
    {
        IMTK_DUMP_FIELDS(STYLE_GENERATOR);
    }
}
