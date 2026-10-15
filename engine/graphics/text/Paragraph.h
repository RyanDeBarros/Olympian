#pragma once

#include "core/base/TransformerExposure.h"

#include "graphics/sprites/Sprite.h"

#include "graphics/text/TextGlyph.h"
#include "graphics/text/TextElement.h"
#include "graphics/text/Typesetting.h"

namespace oly::rendering
{
    class Paragraph;

    struct ParagraphFormatExposure
    {
        friend class Paragraph;

    private:
        Paragraph& paragraph_;

        ParagraphFormatExposure(Paragraph& paragraph);

    public:
        void set_line_spacing(float line_spacing);
        void set_tab_spaces(float tab_spaces);
        void set_linebreak_spacing(float linebreak_spacing);
        void set_pivot(glm::vec2 pivot);
        void set_min_size(glm::vec2 min_size);
        void set_text_wrap(float text_wrap);
        void set_max_height(float max_height);
        void set_padding(glm::vec2 padding);
        void set_horizontal_alignment(HorizontalAlignment alignment);
        void set_vertical_alignment(VerticalAlignment alignment);
    };

    class TextElementExposure;

    namespace internal
    {
        class GlyphGroup
        {
            friend class Paragraph;
            friend class TextElementExposure;

        private:
            struct CachedGlyphInfo
            {
                TypesetData typeset;
                float line_y_offset;
            };

            const Paragraph* paragraph_ = nullptr;
            TextElement element_;

            mutable std::vector<TextGlyph> glyphs_;
            mutable std::vector<CachedGlyphInfo> cached_info_;
            mutable glm::vec2 last_jitter_offset_ = {};
            mutable DirtyGlyphGroup dirty_ = ~DirtyGlyphGroup(0);

        public:
            GlyphGroup(TextElement&& element);

            void set_batch(Unbatched);
            void set_batch(rendering::SpriteBatch& batch);

            void draw() const;

            struct PeekData
            {
                imp::utf::codepoint first_codepoint = imp::utf::codepoint(0);
            };

            PeekData peek() const;
            void build_page_section(TypesetData& typeset, PeekData next_peek) const;

            enum class WriteResult
            {
                Continue,
                Break
            };

            WriteResult write_glyph_section(TypesetData& typeset, PeekData next_peek) const;
            void clear_cache() const;

        private:
            void build_adj_offset(TypesetData& typeset, PeekData next_peek) const;
            void build_space(TypesetData& typeset, imp::utf::codepoint next_codepoint) const;
            void build_tab(TypesetData& typeset, imp::utf::codepoint next_codepoint) const;
            void build_newline(TypesetData& typeset) const;
            void build_glyph(TypesetData& typeset, float dx) const;

            struct LineAlignment
            {
                float y_offset;
            };

            bool write_adj_offset(TypesetData& typeset, PeekData next_peek, LineAlignment& line) const;
            void write_space(TypesetData& typeset, imp::utf::codepoint next_codepoint) const;
            void write_tab(TypesetData& typeset, imp::utf::codepoint next_codepoint) const;
            bool write_newline(TypesetData& typeset, LineAlignment& line) const;
            
            void write_glyph(
                TypesetData& typeset,
                imp::utf::codepoint c,
                float dx,
                LineAlignment line,
                bool is_camera_invariant) const;
            
            glm::vec2 get_glyph_position(size_t i) const;

            float space_width(imp::utf::codepoint next_codepoint) const;
            float tab_width(imp::utf::codepoint next_codepoint) const;

            void recolor() const;
            void realign_lines() const;
            void reposition_jitter() const;

        public:
            void reposition_glyphs() const;
            void set_camera_invariant(bool is_camera_invariant) const;
        };
    }

    struct TextElementExposure
    {
        friend class Paragraph;
        friend class internal::GlyphGroup;

    private:
        Paragraph& paragraph_;
        internal::GlyphGroup& glyph_group_;

        TextElementExposure(Paragraph& paragraph, internal::GlyphGroup& glyph_group);

    public:
        void set_font(const FontAtlasRef& font);
        void set_font(const RasterFontRef& font);
        void set_text(imp::utf::string&& text);
        void set_text_color(glm::vec4 color);
        void set_adj_offset(float adj_offset);
        void set_scale(glm::vec2 scale);
        void set_line_y_pivot(float line_y_pivot);
        void set_jitter_offset(glm::vec2 jitter_offset);
    };

    class Paragraph
    {
        friend class internal::GlyphGroup;
        friend struct ParagraphFormatExposure;
        friend struct TextElementExposure;

    private:
        mutable Sprite bkg_;
        ParagraphFormat format_;

        mutable internal::DirtyParagraph dirty_layout_ = internal::DirtyParagraph::RebuildLayout;

        std::vector<internal::GlyphGroup> glyph_groups_;

        mutable internal::PageBuildData page_data_;
        mutable internal::PageLayout page_layout_;
        mutable internal::AlignmentCache alignment_cache_;
        mutable size_t written_glyph_groups_ = 0;

        mutable Transformer2D transformer_;

    public:
        bool draw_bkg = false;

        Paragraph(std::vector<TextElement>&& elements, const ParagraphFormat& format = {});
        Paragraph(Unbatched, std::vector<TextElement>&& elements, const ParagraphFormat& format = {});
        Paragraph(SpriteBatch& batch, std::vector<TextElement>&& elements, const ParagraphFormat& format = {});
        Paragraph(const Paragraph&);
        Paragraph(Paragraph&&) noexcept;

        Paragraph& operator=(const Paragraph&);
        Paragraph& operator=(Paragraph&&) noexcept;

    public:
        static Paragraph load(TOMLNode node);
        static Paragraph load(TOMLNode node, const DebugTrace& trace);

        internal::SpriteReference::accessor get_batch() const;
        void set_batch(Unbatched);
        void set_batch(SpriteBatch& batch);

        Transformer2DExposure<> get_transformer() const;
        Transformer2DExposure<ex::AttachOnly> set_transformer();

        const ParagraphFormat& get_format() const;
        ParagraphFormatExposure set_format();
        glm::vec4 get_bkg_color() const;
        void set_bkg_color(glm::vec4 color);

        void set_camera_invariant(bool camera_invariant);
        bool is_camera_invariant() const;

        const TextElement& get_element(size_t i = 0) const;
        TextElementExposure set_element(size_t i = 0);
        size_t get_element_count() const;
        void add_element(TextElement&& element);
        void insert_element(size_t i, TextElement&& element);
        void erase_element(size_t i);

        const Transform2D& get_local() const;
        Transform2D& set_local();

        glm::vec2 get_content_size() const;
        glm::vec2 get_fitted_size() const;

        void draw() const;

    private:
        void init(std::vector<TextElement>&& elements);

        void clean_dirty_layout() const;
        void build_layout() const;
        void write_glyphs() const;

        enum AlignmentFlags
        {
            ResizeLines = 1 << 0,
            Vertical = 1 << 1,
            Horizontal = 1 << 2,
            Pivot = 1 << 3,
            Padding = 1 << 4
        };

        void compute_alignment_cache(AlignmentFlags flags) const;

        internal::GlyphGroup::PeekData peek_next(size_t i) const;
        void recompute_content_size_x() const;
        void recompute_content_size_y() const;
        void recompute_fitted_size_x() const;
        void recompute_fitted_size_y() const;
    };

    using ParagraphRef = SmartReference<Paragraph>;
}
