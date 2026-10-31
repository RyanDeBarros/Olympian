#include "RasterFontDocument.h"

#include "assets/TranslateKey.h"

#include "definitions/Keys.h"

namespace oly::editor
{
    static auto MakeGlyphComboName(auto& double_desc)
    {
        return [&double_desc](size_t i) -> std::string {
            auto& desc = double_desc.scratch;
            if (i < desc.glyphs.size() && !desc.glyphs[i].codepoint.value.empty())
                return desc.glyphs[i].codepoint.value;
            else
                return "Glyph #" + std::to_string(i);
        };
    }

    static imtk::w::list_indexer::config GlyphListConfig()
    {
        return {
            .prompt = "Select glyph",
            .create_tooltip = "New glyph",
            .delete_tooltip = "Delete glyph",
            .clear_tooltip = "Clear glyphs"
        };
    }

    RasterFontDocument::RasterFontDocument(detail::ResourcePath oly_path)
        : IDocument(std::move(oly_path))
        , glyphs_(GlyphListConfig(), MakeGlyphComboName(desc_))
    {
    }

    void RasterFontDocument::InitImpl()
    {
        if (!GetOlyPath().is_resource())
            imtk::notify_warning("Asset is not located in resource folder");

        LoadAsset();
    }

    void RasterFontDocument::Draw()
    {
        auto pre_draw = PreDraw();

        imtk::id_scope scope(this);
        Draw(desc_.scratch);
    }

    void RasterFontDocument::LoadImpl()
    {
        if (oly_path_.is_file())
        {
            meta_ = detail::MetaSplitter::decode_meta(oly_path_);

            toml::table table;
            std::string err = oly_path_.load_toml(table);
            if (err.empty())
                Load(imtk::toml_node(table), desc_.disk);
            else
                imtk::notify_error("cannot load raster font - corrupted asset: " + oly_path_.string());

            MarkClean();
        }
        else
        {
            Load(imtk::toml_node(), desc_.disk);

            meta_ = {};
            meta_.map[detail::Key::Meta_Version] = GetVersion();
            meta_.map[detail::Key::Meta_Import] = "0";
            meta_.map[detail::Key::Meta_Type] = detail::encode_key(detail::Key::Meta_RasterFont);

            MarkDirty();
        }

        desc_.load_from_disk();

        codepoint_counter_.clear();
        for (auto& desc : desc_.scratch.glyphs)
            codepoint_counter_.increment(desc.codepoint.value);

        glyphs_.model.init(ListAdapter());
    }

    void RasterFontDocument::DumpImpl()
    {
        toml::table table;
        Dump(table, desc_.scratch);
        oly_path_.dump_toml(table, meta_);
        desc_.write_to_disk();
        MarkClean();
    }

    void RasterFontDocument::ResetAssetImpl()
    {
        Load(imtk::toml_node(), desc_.scratch);
    }

    const imtk::desc::idoubler& RasterFontDocument::GetDoubleDescriptor() const
    {
        return desc_;
    }

    imtk::desc::idoubler& RasterFontDocument::GetDoubleDescriptor()
    {
        return desc_;
    }

    const char* RasterFontDocument::GetVersion()
    {
        return "1.0";
    }

    void RasterFontDocument::Draw(RasterFontDesc& desc)
    {
        if (auto form = imtk::prop::form())
        {
            IMTK_DRAW_FIELDS(RASTER_FONT_PARTIAL_GENERATOR);

            if (auto subform = imtk::prop::subform("Glyphs"))
            {
                if (auto pause = imtk::prop::form::pause())
                {
                    glyphs_.model.sync(ListAdapter());

                    if (auto scope = imtk::id_scope("##Glyph"))
                        glyphs_.draw();
                }

                if (imtk::prop::in_form())
                {
                    if (!desc.glyphs.empty())
                        Draw(desc.glyphs[glyphs_.model.index()]);
                }

                if (glyphs_.model.consume_ops(*ListOpAdapter()))
                    MarkDirty();

                glyphs_.model.consume_index_modified();
            }
        }
    }

    void RasterFontDocument::Draw(GlyphDesc& desc)
    {
        const bool empty_codepoint = desc.codepoint.value.empty();
        const bool duplicate_codepoint = codepoint_counter_.count(desc.codepoint.value) > 1;
        std::string previous_codepoint = desc.codepoint.value;

        imtk::style_stack style_stack;
        if (empty_codepoint || duplicate_codepoint)
        {
            style_stack.push(ImGuiCol_Border, imtk::col::error);
            style_stack.push(ImGuiStyleVar_FrameBorderSize, 1.f);
        }

        {
            auto styles = style_stack.apply();
            desc.codepoint.draw();
        }

        if (imtk::prop::row::dirty())
        {
            codepoint_counter_.increment(desc.codepoint.value);
            codepoint_counter_.decrement(previous_codepoint);
        }


        if (imtk::prop::row::get_draw_result().state.hovered())
        {
            if (empty_codepoint)
                ImGui::SetTooltip("Codepoint is empty");
            else if (duplicate_codepoint)
                ImGui::SetTooltip("Duplicate codepoint");
        }

        IMTK_DRAW_FIELDS(GLYPH_BODY_GENERATOR);
    }

    void RasterFontDocument::Load(imtk::toml_node node, RasterFontDesc& desc)
    {
        IMTK_LOAD_FIELDS(RASTER_FONT_PARTIAL_GENERATOR);

        desc.glyphs.clear();
        if (auto array = desc.glyphs.subnode(node).as_array())
        {
            for (size_t i = 0; i < array->size(); ++i)
            {
                desc.glyphs.push_back();
                Load(imtk::toml_node(array->get(i)), desc.glyphs.back());
            }
        }
    }

    void RasterFontDocument::Load(imtk::toml_node node, GlyphDesc& desc)
    {
        IMTK_LOAD_FIELDS(GLYPH_GENERATOR);
    }

    void RasterFontDocument::Dump(toml::table& table, RasterFontDesc& desc)
    {
        IMTK_DUMP_FIELDS(RASTER_FONT_PARTIAL_GENERATOR);

        toml::array array;
        array.reserve(desc.glyphs.size());
        for (auto& subdesc : desc.glyphs)
            Dump(array.emplace_back<toml::table>(), subdesc);
        desc.glyphs.dump_into(table, std::move(array));
    }

    void RasterFontDocument::Dump(toml::table& table, GlyphDesc& desc)
    {
        IMTK_DUMP_FIELDS(GLYPH_GENERATOR);
    }

    imtk::list_adapter RasterFontDocument::ListAdapter()
    {
        return {
            .sized = std::make_unique<imtk::vector_sized<GlyphDesc>>(desc_.scratch.glyphs),
            .ops = ListOpAdapter()
        };
    }

    std::unique_ptr<imtk::ilist_op_adapter> RasterFontDocument::ListOpAdapter()
    {
        auto callback = imtk::make_counter_callback(codepoint_counter_,
            [this](size_t i) -> const std::string& { return desc_.scratch.glyphs[i].codepoint.value; });
        return std::make_unique<imtk::list_callback_adapter>(
            std::make_unique<imtk::vector_op_adapter<GlyphDesc, GlyphDesc::Printer>>(desc_.scratch.glyphs),
            std::move(callback));
    }
}
