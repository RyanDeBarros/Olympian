#include "FontDocument.h"

#include "assets/TranslateKey.h"
#include "definitions/Keys.h"

#include <imp/hash.hpp>

namespace oly::editor
{
    static imtk::w::list_indexer::config AtlasListConfig()
    {
        return {
            .prompt = "Select atlas",
            .create_tooltip = "Create atlas",
            .delete_tooltip = "Delete atlas",
            .clear_tooltip = "Clear atlases"
        };
    }

    FontDocument::FontDocument(detail::ResourcePath oly_path)
        : IDocument(std::move(oly_path))
        , atlas_slots_(AtlasListConfig(), "Atlas")
    {
    }

    void FontDocument::InitImpl()
    {
        if (!GetSourcePath().is_resource())
            imtk::notify_warning("Asset is not located in resource folder");

        atlas_slots_.model.policy = imtk::list_policy::minimum_one;
        display_text_.value = "Abc 123";
        display_text_.config().label = "Display text";
        LoadAsset();
    }

    void FontDocument::Draw()
    {
        auto pre_draw = PreDraw();

        imtk::id_scope scope(this);

        if (auto _ = imtk::tab_bar(""))
        {
            if (auto _ = imtk::tab_item("Font Face"))
                DrawFontFace();

            if (auto _ = imtk::tab_item("Font Atlases"))
                DrawFontAtlases();
        }
    }

    void FontDocument::LoadImpl()
    {
        if (oly_path_.is_file())
        {
            meta_ = detail::MetaSplitter::decode_meta(oly_path_);

            toml::table table;
            std::string err = oly_path_.load_toml(table);
            if (err.empty())
                Load(imtk::toml_node(table), desc_.disk);
            else
                imtk::notify_error("cannot load font - corrupted asset: " + GetSourcePath().string());

            MarkClean();
        }
        else
        {
            Load(imtk::toml_node(), desc_.disk);

            meta_ = {};
            meta_.map[detail::Key::Meta_Version] = GetVersion();
            meta_.map[detail::Key::Meta_Import] = "1";
            meta_.map[detail::Key::Meta_Type] = detail::encode_key(detail::Key::Meta_Font);

            MarkDirty();
        }

        desc_.load_from_disk();

        atlas_slots_.model.init(imtk::make_vector_adapter<FontAtlasDesc::Printer>(desc_.scratch.font_atlases));
    }

    void FontDocument::DumpImpl()
    {
        toml::table table;
        Dump(table, desc_.scratch);
        oly_path_.dump_toml(table, meta_);
        desc_.write_to_disk();
        MarkClean();
    }

    void FontDocument::ResetAssetImpl()
    {
        Load(imtk::toml_node(), desc_.scratch);
    }

    const imtk::desc::idoubler& FontDocument::GetDoubleDescriptor() const
    {
        return desc_;
    }

    imtk::desc::idoubler& FontDocument::GetDoubleDescriptor()
    {
        return desc_;
    }

    const char* FontDocument::GetVersion()
    {
        return "1.0";
    }

    detail::ResourcePath FontDocument::GetSourcePath() const
    {
        return oly_path_.get_source_path();
    }

    void FontDocument::DrawFontFace()
    {
        if (auto form = imtk::prop::form())
            Draw(*desc_.scratch.font_face);
    }

    void FontDocument::DrawFontAtlases()
    {
        if (auto _ = imtk::table("", 2))
        {
            ImGui::TableNextColumn();

            atlas_slots_.model.sync(imtk::make_vector_adapter<FontAtlasDesc::Printer>(desc_.scratch.font_atlases));
            if (auto scope = imtk::id_scope("##Atlas"))
                atlas_slots_.draw();

            if (auto form = imtk::prop::form())
            {
                if (!desc_.scratch.font_atlases.empty())
                    Draw(desc_.scratch.font_atlases[atlas_slots_.model.index()]);

                if (atlas_slots_.model.consume_ops(
                    imtk::make_vector_op_adapter<FontAtlasDesc::Printer>(desc_.scratch.font_atlases)))
                {
                    MarkDirty();
                }

                if (atlas_slots_.model.consume_index_modified())
                    preview_font_.reset();
            }

            ImGui::TableNextColumn();
            DrawAtlasPreview();
        }
    }

    void FontDocument::DrawAtlasPreview()
    {
        if (auto _ = imtk::child("Preview", ImVec2(0, 0), ImGuiChildFlags_Borders))
        {
            ImGui::TextUnformatted("Preview");
            ImGui::Separator();

            display_text_.draw();

            if (!preview_font_)
            {
                preview_font_ = imtk::font_instance(
                    GetSourcePath().string().c_str(),
                    desc_.scratch.font_atlases[atlas_slots_.model.index()].font_size.value);
            }

            if (auto _ = imtk::font_scope(preview_font_))
                ImGui::TextUnformatted(display_text_.value.c_str());
        }
    }

    void FontDocument::Draw(FontFaceDesc& desc)
    {
        desc.storage.draw();

        glyph_counter_.clear();
        for (auto& k : desc.kerning)
            glyph_counter_.increment({ k.pair.fields[0].edit.buffer(), k.pair.fields[1].edit.buffer() });

        if (!desc.kerning_widget.body.row_draw)
        {
            desc.kerning_widget.body.row_draw = [this, &desc](imtk::dynamic_row& row) -> imtk::item_result {
                imtk::w::widget_row components;
                auto& k = desc.kerning[row.index()];

                bool dup_warning = glyph_counter_.count(
                    { k.pair.fields[0].edit.buffer(), k.pair.fields[1].edit.buffer() }) > 1;

                imtk::outline dup_outline;
                for (size_t i = 0; i < 2; ++i)
                {
                    components.subwidgets.push_back(
                        std::make_unique<imtk::w::generic_widget>(
                            [&k, i, &dup_warning, &dup_outline]() -> imtk::item_result {
                                bool bad_codepoint = !imp::stocdpt(k.pair.fields[i].edit.buffer()).has_value();
                                if (bad_codepoint)
                                    dup_warning = false;

                                imtk::item_result result;

                                if (i == 0)
                                {
                                    ImGui::TextUnformatted(k.pair.label.c_str());
                                    result |= imtk::item_result::query(false);
                                    ImGui::SameLine();
                                }

                                imtk::outline bad_outline;
                                result |= imtk::w::bound_widget<std::string>(k.pair.fields[i].edit.buffer()).draw();

                                if (dup_warning && result.state.hovered())
                                    ImGui::SetTooltip("Duplicate codepoint pair");

                                if (bad_codepoint)
                                {
                                    if (result.state.hovered())
                                        ImGui::SetTooltip("Bad codepoint format");

                                    bad_outline.draw(imtk::col::error);
                                }

                                if (i == 1)
                                {
                                    if (dup_warning)
                                        dup_outline.draw(imtk::col::error);
                                }

                                return result;
                            }));
                }

                components.subwidgets.push_back(std::make_unique<imtk::w::generic_widget>([&k]() -> imtk::item_result {
                    imtk::controls::vertical_separator();
                    ImGui::TextUnformatted(k.distance.label.c_str());
                    auto result = imtk::item_result::query(false);
                    ImGui::SameLine();
                    result |= imtk::w::bound_widget<int>(k.distance.edit.buffer()).draw();
                    return result;
                }));

                auto result = components.draw();
                k.pair.fields[0].edit.post_edit(result.state);
                k.pair.fields[1].edit.post_edit(result.state);
                k.distance.edit.post_edit(result.state);
                return result;
            };
        }

        std::vector<std::unique_ptr<imtk::prop::iresettable>> resetters;
        resetters.push_back(
            std::make_unique<imtk::prop::resettable_vector_size<KerningDesc>>(
                desc.kerning_widget.model,
                desc.kerning,
                0));

        for (auto& k : desc.kerning)
            resetters.push_back(k.make_resetter());

        if (auto _ = imtk::prop::multi_row_scope("Kerning", std::move(resetters)))
        {
            imtk::prop::value::add_component(
                std::make_unique<imtk::w::bound_dynamic_list>(desc.kerning_widget, desc.kerning.size()));
        }

        desc.kerning.consume_ops(desc.kerning_widget.model);

        for (size_t i = 0; i < desc.kerning.size(); ++i)
            desc.kerning[i].check_undo_action();
    }

    void FontDocument::Draw(FontAtlasDesc& desc)
    {
        desc.font_size.draw();
        if (imtk::prop::row::dirty())
            preview_font_.reset();

        IMTK_DRAW_FIELDS(FONT_ATLAS_NONPREVIEW_GENERATOR);

        if (auto subform = imtk::prop::subform("Common buffer"))
        {
            desc.use_common_buffer_preset.draw();
            bool preset = desc.use_common_buffer_preset.value;

            if (auto d = imtk::disabled(!preset))
            {
                desc.common_buffer_preset.draw();
                if (auto scope = imtk::id_scope(&desc.common_buffer_preset))
                {
                    imtk::prop::value::add_component(
                        std::make_unique<imtk::w::readonly_text_owned>(
                            detail::buffer_of(desc.common_buffer_preset.value)));
                    imtk::prop::row::submit();
                }
            }

            if (auto d = imtk::disabled(preset))
                desc.common_buffer.draw();
        }
    }

    void FontDocument::Load(imtk::toml_node node, FullFontDesc& desc)
    {
        Load(desc.font_face.subnode(node), *desc.font_face);

        const toml::array* array = desc.font_atlases.subnode(node).as_array();
        if (array && !array->empty())
        {
            for (size_t i = 0; i < array->size(); ++i)
            {
                desc.font_atlases.push_back();
                Load(imtk::toml_node(*array->get(i)), desc.font_atlases.back());
            }
        }
        else
        {
            desc.font_atlases.push_back();
            Load(imtk::toml_node(), desc.font_atlases.back());
        }
    }

    void FontDocument::Load(imtk::toml_node node, FontFaceDesc& desc)
    {
        desc.storage.load(node);

        const toml::array* array = desc.kerning.subnode(node).as_array();
        if (array && !array->empty())
        {
            for (size_t i = 0; i < array->size(); ++i)
            {
                desc.kerning.push_back();
                Load(imtk::toml_node(*array->get(i)), desc.kerning.back());
            }
        }
    }

    void FontDocument::Load(imtk::toml_node node, KerningDesc& desc)
    {
        IMTK_LOAD_FIELDS(KERNING_GENERATOR);
    }

    void FontDocument::Load(imtk::toml_node node, FontAtlasDesc& desc)
    {
        IMTK_LOAD_FIELDS(FONT_ATLAS_GENERATOR);
    }

    void FontDocument::Dump(toml::table& table, FullFontDesc& desc)
    {
        toml::table subtable;
        Dump(subtable, *desc.font_face);
        desc.font_face.dump_into(table, std::move(subtable));

        toml::array array;
        for (auto& d : desc.font_atlases)
            Dump(array.emplace_back<toml::table>(), d);
        desc.font_atlases.dump_into(table, std::move(array));
    }

    void FontDocument::Dump(toml::table& table, FontFaceDesc& desc)
    {
        desc.storage.dump(table);

        toml::array array;
        for (auto& d : desc.kerning)
            Dump(array.emplace_back<toml::table>(), d);
        desc.kerning.dump_into(table, std::move(array));
    }

    void FontDocument::Dump(toml::table& table, KerningDesc& desc)
    {
        IMTK_DUMP_FIELDS(KERNING_GENERATOR);
    }

    void FontDocument::Dump(toml::table& table, FontAtlasDesc& desc)
    {
        IMTK_DUMP_FIELDS(FONT_ATLAS_GENERATOR);
    }
}
