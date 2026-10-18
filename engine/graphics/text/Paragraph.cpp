#include "Paragraph.h"

#include "core/base/Errors.h"

#include "core/context/rendering/Fonts.h"
#include "core/context/rendering/Sprites.h"
#include "core/context/rendering/Textures.h"

#include "core/util/Parser.h"
#include "core/util/Loader.h"

#include "graphics/resources/Textures.h"

#include "definitions/Keys.h"

namespace oly::rendering
{
    ParagraphFormatExposure::ParagraphFormatExposure(Paragraph& paragraph)
        : paragraph_(paragraph)
    {
    }

    void ParagraphFormatExposure::set_line_spacing(float line_spacing)
    {
        if (paragraph_.format_.line_spacing != line_spacing)
        {
            paragraph_.format_.line_spacing = line_spacing;
            if (paragraph_.format_.max_height > 0.0f)
                paragraph_.dirty_layout_ |= internal::DirtyParagraph::RebuildLayout;
            else
                paragraph_.dirty_layout_ |= internal::DirtyParagraph::LineSpacing;
        }
    }

    void ParagraphFormatExposure::set_tab_spaces(float tab_spaces)
    {
        if (paragraph_.format_.tab_spaces != tab_spaces)
        {
            paragraph_.format_.tab_spaces = tab_spaces;
            paragraph_.dirty_layout_ |= internal::DirtyParagraph::RebuildLayout;
        }
    }

    void ParagraphFormatExposure::set_linebreak_spacing(float linebreak_spacing)
    {
        if (paragraph_.format_.linebreak_spacing != linebreak_spacing)
        {
            paragraph_.format_.linebreak_spacing = linebreak_spacing;
            if (paragraph_.format_.max_height > 0.0f)
                paragraph_.dirty_layout_ |= internal::DirtyParagraph::RebuildLayout;
            else
                paragraph_.dirty_layout_ |= internal::DirtyParagraph::LineSpacing;
        }
    }

    void ParagraphFormatExposure::set_pivot(glm::vec2 pivot)
    {
        if (paragraph_.format_.pivot != pivot)
        {
            paragraph_.format_.pivot = pivot;
            paragraph_.dirty_layout_ |= internal::DirtyParagraph::Pivot;
        }
    }

    void ParagraphFormatExposure::set_min_size(glm::vec2 min_size)
    {
        if (paragraph_.format_.min_size != min_size)
        {
            paragraph_.format_.min_size = min_size;
            paragraph_.dirty_layout_ |= internal::DirtyParagraph::MinSize;
        }
    }

    void ParagraphFormatExposure::set_text_wrap(float text_wrap)
    {
        if (paragraph_.format_.text_wrap != text_wrap)
        {
            paragraph_.format_.text_wrap = text_wrap;
            paragraph_.dirty_layout_ |= internal::DirtyParagraph::RebuildLayout;
        }
    }

    void ParagraphFormatExposure::set_max_height(float max_height)
    {
        if (paragraph_.format_.max_height != max_height)
        {
            paragraph_.format_.max_height = max_height;
            paragraph_.dirty_layout_ |= internal::DirtyParagraph::RebuildLayout;
        }
    }

    void ParagraphFormatExposure::set_padding(glm::vec2 padding)
    {
        if (paragraph_.format_.padding != padding)
        {
            paragraph_.format_.padding = padding;
            paragraph_.dirty_layout_ |= internal::DirtyParagraph::Padding;
        }
    }

    void ParagraphFormatExposure::set_horizontal_alignment(HorizontalAlignment alignment)
    {
        if (paragraph_.format_.horizontal_alignment != alignment)
        {
            paragraph_.format_.horizontal_alignment = alignment;
            paragraph_.dirty_layout_ |= internal::DirtyParagraph::HorizontalAlign;
        }
    }

    void ParagraphFormatExposure::set_vertical_alignment(VerticalAlignment alignment)
    {
        if (paragraph_.format_.vertical_alignment != alignment)
        {
            paragraph_.format_.vertical_alignment = alignment;
            paragraph_.dirty_layout_ |= internal::DirtyParagraph::VerticalAlign;
        }
    }

    internal::GlyphGroup::GlyphGroup(TextElement&& element)
        : element_(std::move(element))
    {
    }

    void internal::GlyphGroup::set_batch(Unbatched)
    {
        for (auto& glyph : glyphs_)
            glyph.set_batch(kUnbatched);
    }

    void internal::GlyphGroup::set_batch(rendering::SpriteBatch& batch)
    {
        for (auto& glyph : glyphs_)
            glyph.set_batch(batch);
    }

    void internal::GlyphGroup::draw() const
    {
        if (dirty_ & DirtyGlyphGroup::LineAlignment)
            realign_lines();

        if (dirty_ & DirtyGlyphGroup::JitterOffset)
            reposition_jitter();

        if (dirty_ & DirtyGlyphGroup::Recolor)
            recolor();

        for (const TextGlyph& glyph : glyphs_)
            glyph.draw();
    }

    internal::GlyphGroup::PeekData internal::GlyphGroup::peek() const
    {
        auto iter = element_.text.begin();
        return { .first_codepoint = iter ? *iter : imp::utf::codepoint(0) };
    }

    void internal::GlyphGroup::build_page_section(TypesetData& typeset, PeekData next_peek) const
    {
        auto iter = element_.text.begin();
        if (!iter)
            return;

        paragraph_->page_data_.current_line().max_height = glm::max(
            paragraph_->page_data_.current_line().max_height,
            element_.line_height());

        build_adj_offset(typeset, next_peek);

        while (iter)
        {
            imp::utf::codepoint codepoint = iter.advance();
            imp::utf::codepoint next_codepoint = iter ? *iter : next_peek.first_codepoint;

            if (codepoint == ' ')
                build_space(typeset, next_codepoint);
            else if (codepoint == '\t')
                build_tab(typeset, next_codepoint);
            else if (imp::utf::is_n_or_r(codepoint))
            {
                if (iter && imp::utf::is_rn(codepoint, next_codepoint))
                    ++iter;
                build_newline(typeset);
                if (iter || next_peek.first_codepoint) // next codepoint in group
                    paragraph_->page_data_.current_line().max_height = element_.line_height();
            }
            else if (element_.font.support(codepoint))
            {
                float dx = element_.advance_width(codepoint, next_codepoint);
                if (!paragraph_->format_.can_fit_on_line(typeset, dx))
                    build_newline(typeset);
                build_glyph(typeset, dx);
            }
            else
            {
                _OLY_ENGINE_LOG_WARNING("RENDERING") << "Font does not support the glyph with codepoint ("
                    << codepoint << ")." << kLog.nl;
            }
        }
    }

    internal::GlyphGroup::WriteResult internal::GlyphGroup::write_glyph_section(
        TypesetData& typeset,
        PeekData next_peek) const
    {
        dirty_ = internal::DirtyGlyphGroup(0);
        clear_cache();

        auto iter = element_.text.begin();
        if (!iter)
            return WriteResult::Continue;

        LineAlignment line{ .y_offset = element_.line_y_pivot * (
            paragraph_->page_data_.lines[typeset.line].max_height - element_.line_height()) };

        if (!write_adj_offset(typeset, next_peek, line))
            return WriteResult::Break;

        while (iter)
        {
            imp::utf::codepoint codepoint = iter.advance();
            imp::utf::codepoint next_codepoint = iter ? *iter : next_peek.first_codepoint;

            if (codepoint == ' ')
                write_space(typeset, next_codepoint);
            else if (codepoint == '\t')
                write_tab(typeset, next_codepoint);
            else if (imp::utf::is_n_or_r(codepoint))
            {
                if (iter && imp::utf::is_rn(codepoint, next_codepoint))
                    ++iter;
                if (!write_newline(typeset, line))
                    return WriteResult::Break;
            }
            else if (element_.font.support(codepoint))
            {
                float dx = element_.advance_width(codepoint, next_codepoint);
                if (!paragraph_->format_.can_fit_on_line(typeset, dx))
                {
                    if (!write_newline(typeset, line))
                        return WriteResult::Break;
                }
                write_glyph(typeset, codepoint, dx, line, paragraph_->is_camera_invariant());
            }
            else
            {
                _OLY_ENGINE_LOG_WARNING("RENDERING") << "Font does not support the glyph with codepoint ("
                    << codepoint << ")." << kLog.nl;
            }
        }

        recolor();
        return WriteResult::Continue;
    }

    void internal::GlyphGroup::clear_cache() const
    {
        glyphs_.clear();
        cached_info_.clear();
    }

    void internal::GlyphGroup::build_adj_offset(TypesetData& typeset, PeekData next_peek) const
    {
        if (element_.adj_offset <= 0.0f || typeset.x == 0.0f)
            return;

        auto iter = element_.text.begin();
        const imp::utf::codepoint codepoint = iter.advance();
        if (imp::utf::is_n_or_r(codepoint))
            return;

        const imp::utf::codepoint next_codepoint = iter ? *iter : next_peek.first_codepoint;
        float dx = 0.0f;
        if (codepoint == ' ')
            dx = space_width(next_codepoint);
        else if (codepoint == '\t')
            dx = tab_width(next_codepoint);
        else if (element_.font.support(codepoint))
            dx = element_.advance_width(codepoint, next_codepoint);
        else
        {
            _OLY_ENGINE_LOG_WARNING("RENDERING") << "Font does not support the glyph with codepoint ("
                << codepoint << ")." << kLog.nl;
        }

        if (paragraph_->format_.can_fit_on_line(typeset, element_.adj_offset + dx))
        {
            typeset.x += element_.adj_offset;
            paragraph_->page_data_.current_line().space_width += element_.adj_offset;
            ++paragraph_->page_data_.current_line().characters;
        }
        else
        {
            build_newline(typeset);
            if (iter || next_peek.first_codepoint) // next codepoint in group
                paragraph_->page_data_.current_line().max_height = element_.line_height();
        }
    }

    void internal::GlyphGroup::build_space(TypesetData& typeset, imp::utf::codepoint next_codepoint) const
    {
        const float dx = space_width(next_codepoint);
        typeset.x += dx;
        paragraph_->page_data_.current_line().space_width += dx;
        ++paragraph_->page_data_.current_line().characters;
    }

    void internal::GlyphGroup::build_tab(TypesetData& typeset, imp::utf::codepoint next_codepoint) const
    {
        const float dx = tab_width(next_codepoint);
        typeset.x += dx;
        paragraph_->page_data_.current_line().space_width += dx;
        ++paragraph_->page_data_.current_line().characters;
    }

    void internal::GlyphGroup::build_newline(TypesetData& typeset) const
    {
        if (typeset.x == 0.0f)
            ++paragraph_->page_data_.linebreaks;

        paragraph_->page_data_.current_line().width = typeset.x;
        ++typeset.line;
        typeset.y -= paragraph_->page_data_.current_line().spaced_height(paragraph_->format_);
        typeset.x = 0.0f;
        paragraph_->page_data_.lines.emplace_back();
    }

    void internal::GlyphGroup::build_glyph(TypesetData& typeset, float dx) const
    {
        typeset.x += dx;
        paragraph_->page_data_.current_line().final_advance_width = dx;
        ++paragraph_->page_data_.current_line().characters;
    }

    bool internal::GlyphGroup::write_adj_offset(TypesetData& typeset, PeekData next_peek, LineAlignment& line) const
    {
        if (element_.adj_offset <= 0.0f || typeset.x == 0.0f)
            return true;

        auto iter = element_.text.begin();
        const imp::utf::codepoint codepoint = iter.advance();
        if (imp::utf::is_n_or_r(codepoint))
            return true;

        const imp::utf::codepoint next_codepoint = iter ? *iter : next_peek.first_codepoint;
        float dx = 0.0f;
        if (codepoint == ' ')
            dx = space_width(next_codepoint);
        else if (codepoint == '\t')
            dx = tab_width(next_codepoint);
        else if (element_.font.support(codepoint))
            dx = element_.advance_width(codepoint, next_codepoint);
        else
        {
            _OLY_ENGINE_LOG_WARNING("RENDERING") << "Font does not support the glyph with codepoint ("
                << codepoint << ")." << kLog.nl;
        }

        if (paragraph_->format_.can_fit_on_line(typeset, element_.adj_offset + dx))
        {
            typeset.x += element_.adj_offset * paragraph_->alignment_cache_.lines[typeset.line].space_width_mult;
            ++typeset.character;
            return true;
        }
        else
            return write_newline(typeset, line);
    }

    void internal::GlyphGroup::write_space(TypesetData& typeset, imp::utf::codepoint next_codepoint) const
    {
        typeset.x += space_width(next_codepoint) * paragraph_->alignment_cache_.lines[typeset.line].space_width_mult;
        ++typeset.character;
    }

    void internal::GlyphGroup::write_tab(TypesetData& typeset, imp::utf::codepoint next_codepoint) const
    {
        typeset.x += tab_width(next_codepoint) * paragraph_->alignment_cache_.lines[typeset.line].space_width_mult;
        ++typeset.character;
    }

    bool internal::GlyphGroup::write_newline(TypesetData& typeset, LineAlignment& line) const
    {
        const float dy = paragraph_->alignment_cache_.lines[typeset.line].height;
        if (!paragraph_->format_.can_fit_vertically(typeset, dy))
            return false;

        ++typeset.line;
        typeset.y -= dy;
        typeset.x = 0.0f;
        typeset.character = 0;

        if (typeset.line < paragraph_->alignment_cache_.lines.size())
        {
            line.y_offset = element_.line_y_pivot * (
                paragraph_->page_data_.lines[typeset.line].max_height - element_.line_height());
        }

        return true;
    }

    void internal::GlyphGroup::write_glyph(
        TypesetData& typeset,
        imp::utf::codepoint c,
        float dx,
        LineAlignment line,
        bool is_camera_invariant) const
    {
        cached_info_.push_back(CachedGlyphInfo{ .typeset = typeset, .line_y_offset = line.y_offset });
        TextGlyph glyph;
        glyph.transformer.attach_parent(&paragraph_->transformer_);
        glyph.set_camera_invariant(is_camera_invariant);
        element_.set_glyph(glyph, c, get_glyph_position(glyphs_.size()));
        glyphs_.push_back(std::move(glyph));
        typeset.x += dx;
        ++typeset.character;
    }

    glm::vec2 internal::GlyphGroup::get_glyph_position(size_t i) const
    {
        const CachedGlyphInfo& cache = cached_info_[i];
        return paragraph_->alignment_cache_.position(cache.typeset)
                + glm::vec2{ 0.0f, cache.line_y_offset }
                + element_.jitter_offset;
    }

    float internal::GlyphGroup::space_width(imp::utf::codepoint next_codepoint) const
    {
        return element_.advance_width(imp::utf::codepoint(' '), next_codepoint);
    }

    float internal::GlyphGroup::tab_width(imp::utf::codepoint next_codepoint) const
    {
        return space_width(imp::utf::codepoint(' ')) * (paragraph_->format_.tab_spaces - 1.0f)
                + space_width(next_codepoint);
    }

    void internal::GlyphGroup::recolor() const
    {
        dirty_ &= ~DirtyGlyphGroup::Recolor;
        for (TextGlyph& glyph : glyphs_)
            glyph.set_text_color(element_.text_color);
    }

    void internal::GlyphGroup::realign_lines() const
    {
        dirty_ &= ~DirtyGlyphGroup::LineAlignment;

        float new_line_y_offset = 0.0f;
        size_t last_line = -1;

        for (size_t i = 0; i < glyphs_.size(); ++i)
        {
            CachedGlyphInfo& info = cached_info_[i];

            if (info.typeset.line != last_line)
            {
                new_line_y_offset = element_.line_y_pivot * (
                    paragraph_->page_data_.lines[info.typeset.line].max_height - element_.line_height());
            }

            if (info.line_y_offset != new_line_y_offset)
            {
                TextGlyph& glyph = glyphs_[i];
                glyph.set_local().position.y -= info.line_y_offset;
                info.line_y_offset = new_line_y_offset;
                glyph.set_local().position.y += info.line_y_offset;
            }
        }
    }

    void internal::GlyphGroup::reposition_jitter() const
    {
        dirty_ &= ~DirtyGlyphGroup::JitterOffset;
        for (TextGlyph& glyph : glyphs_)
            glyph.set_local().position += element_.jitter_offset - last_jitter_offset_;
    }

    void internal::GlyphGroup::reposition_glyphs() const
    {
        for (size_t i = 0; i < glyphs_.size(); ++i)
            glyphs_[i].set_local().position = get_glyph_position(i);
    }

    void internal::GlyphGroup::set_camera_invariant(bool is_camera_invariant) const
    {
        for (TextGlyph& glyph : glyphs_)
            glyph.set_camera_invariant(is_camera_invariant);
    }

    TextElementExposure::TextElementExposure(Paragraph& paragraph, internal::GlyphGroup& glyph_group)
        : paragraph_(paragraph)
        , glyph_group_(glyph_group)
    {
    }

    void TextElementExposure::set_font(const FontAtlasRef& font)
    {
        if (glyph_group_.element_.font != font)
        {
            glyph_group_.element_.font = font;
            paragraph_.dirty_layout_ |= internal::DirtyParagraph::RebuildLayout;
        }
    }

    void TextElementExposure::set_font(const RasterFontRef& font)
    {
        if (glyph_group_.element_.font != font)
        {
            glyph_group_.element_.font = font;
            paragraph_.dirty_layout_ |= internal::DirtyParagraph::RebuildLayout;
        }
    }

    void TextElementExposure::set_text(imp::utf::string&& text)
    {
        if (glyph_group_.element_.text != text)
        {
            glyph_group_.element_.text = std::move(text);
            paragraph_.dirty_layout_ |= internal::DirtyParagraph::RebuildLayout;
        }
    }

    void TextElementExposure::set_text_color(glm::vec4 color)
    {
        if (glyph_group_.element_.text_color != color)
        {
            glyph_group_.element_.text_color = color;
            glyph_group_.dirty_ |= internal::DirtyGlyphGroup::Recolor;
        }
    }

    void TextElementExposure::set_adj_offset(float adj_offset)
    {
        if (glyph_group_.element_.adj_offset != adj_offset)
        {
            glyph_group_.element_.adj_offset = adj_offset;
            paragraph_.dirty_layout_ |= internal::DirtyParagraph::RebuildLayout;
        }
    }

    void TextElementExposure::set_scale(glm::vec2 scale)
    {
        if (glyph_group_.element_.scale != scale)
        {
            glyph_group_.element_.scale = scale;
            paragraph_.dirty_layout_ |= internal::DirtyParagraph::RebuildLayout;
        }
    }

    void TextElementExposure::set_line_y_pivot(float line_y_pivot)
    {
        if (glyph_group_.element_.line_y_pivot != line_y_pivot)
        {
            glyph_group_.element_.line_y_pivot = line_y_pivot;
            glyph_group_.dirty_ |= internal::DirtyGlyphGroup::LineAlignment;
        }
    }

    void TextElementExposure::set_jitter_offset(glm::vec2 jitter_offset)
    {
        if (glyph_group_.dirty_ & internal::DirtyGlyphGroup::JitterOffset)
            glyph_group_.element_.jitter_offset = jitter_offset;
        else if (glyph_group_.element_.jitter_offset != jitter_offset)
        {
            glyph_group_.dirty_ |= internal::DirtyGlyphGroup::JitterOffset;
            glyph_group_.last_jitter_offset_ = glyph_group_.element_.jitter_offset;
            glyph_group_.element_.jitter_offset = jitter_offset;
        }
    }

    Paragraph::Paragraph(std::vector<TextElement>&& elements, const ParagraphFormat& format)
        : format_(format)
        , bkg_()
    {
        init(std::move(elements));
    }

    Paragraph::Paragraph(Unbatched, std::vector<TextElement>&& elements, const ParagraphFormat& format)
        : format_(format)
        , bkg_(kUnbatched)
    {
        init(std::move(elements));
    }

    Paragraph::Paragraph(SpriteBatch& batch, std::vector<TextElement>&& elements, const ParagraphFormat& format)
        : format_(format)
        , bkg_(batch)
    {
        init(std::move(elements));
    }

    Paragraph::Paragraph(const Paragraph& o)
        : bkg_(o.bkg_)
        , format_(o.format_)
        , dirty_layout_(o.dirty_layout_)
        , glyph_groups_(o.glyph_groups_)
        , page_data_(o.page_data_)
        , page_layout_(o.page_layout_)
        , transformer_(o.transformer_)
        , draw_bkg(o.draw_bkg)
    {
        bkg_.transformer.attach_parent(&transformer_);

        for (internal::GlyphGroup& glyph_group : glyph_groups_)
            glyph_group.paragraph_ = this;
    }

    Paragraph::Paragraph(Paragraph&& o) noexcept
        : bkg_(std::move(o.bkg_))
        , format_(std::move(o.format_))
        , dirty_layout_(o.dirty_layout_)
        , glyph_groups_(std::move(o.glyph_groups_))
        , page_data_(std::move(o.page_data_))
        , page_layout_(std::move(o.page_layout_))
        , transformer_(std::move(o.transformer_))
        , draw_bkg(o.draw_bkg)
    {
        for (internal::GlyphGroup& glyph_group : glyph_groups_)
            glyph_group.paragraph_ = this;
    }

    Paragraph& Paragraph::operator=(const Paragraph& o)
    {
        if (this != &o)
            *this = imp::dupl(o);

        return *this;
    }

    Paragraph& Paragraph::operator=(Paragraph&& o) noexcept
    {
        if (this != &o)
        {
            bkg_ = std::move(o.bkg_);
            format_ = std::move(o.format_);
            dirty_layout_ = o.dirty_layout_;
            glyph_groups_ = std::move(o.glyph_groups_);
            page_data_ = std::move(o.page_data_);
            page_layout_ = std::move(o.page_layout_);
            transformer_ = std::move(o.transformer_);
            draw_bkg = o.draw_bkg;

            for (internal::GlyphGroup& glyph_group_ : glyph_groups_)
                glyph_group_.paragraph_ = this;
        }
        return *this;
    }

    internal::SpriteReference::accessor Paragraph::get_batch() const
    {
        return bkg_.get_batch();
    }

    void Paragraph::set_batch(Unbatched)
    {
        bkg_.set_batch(kUnbatched);
        for (auto& glyph_group_ : glyph_groups_)
            glyph_group_.set_batch(kUnbatched);
    }

    void Paragraph::set_batch(SpriteBatch& batch)
    {
        bkg_.set_batch(batch);
        for (auto& glyph_group_ : glyph_groups_)
            glyph_group_.set_batch(batch);
    }

    Transformer2DExposure<> Paragraph::get_transformer() const
    {
        return transformer_;
    }

    Transformer2DExposure<ex::AttachOnly> Paragraph::set_transformer()
    {
        return transformer_;
    }

    const ParagraphFormat& Paragraph::get_format() const
    {
        return format_;
    }

    ParagraphFormatExposure Paragraph::set_format()
    {
        return ParagraphFormatExposure(*this);
    }

    glm::vec4 Paragraph::get_bkg_color() const
    {
        return bkg_.get_modulation();
    }

    void Paragraph::set_bkg_color(glm::vec4 color)
    {
        bkg_.set_modulation(color);
    }

    void Paragraph::set_camera_invariant(bool camera_invariant)
    {
        bkg_.set_camera_invariant(camera_invariant);
        for (auto& glyph_group : glyph_groups_)
            glyph_group.set_camera_invariant(camera_invariant);
    }

    bool Paragraph::is_camera_invariant() const
    {
        return bkg_.is_camera_invariant();
    }

    const TextElement& Paragraph::get_element(size_t i) const
    {
        return glyph_groups_[i].element_;
    }
    
    TextElementExposure Paragraph::set_element(size_t i)
    {
        return TextElementExposure(*this, glyph_groups_[i]);
    }

    size_t Paragraph::get_element_count() const
    {
        return glyph_groups_.size();
    }

    void Paragraph::add_element(TextElement&& element)
    {
        glyph_groups_.emplace_back(std::move(element));
        glyph_groups_.back().paragraph_ = this;
        dirty_layout_ |= internal::DirtyParagraph::RebuildLayout;
    }

    void Paragraph::insert_element(size_t i, TextElement&& element)
    {
        if (i > glyph_groups_.size())
            throw Error(ErrorCode::IndexOutOfRange);

        glyph_groups_.emplace(glyph_groups_.begin() + i, std::move(element));
        glyph_groups_[i].paragraph_ = this;
        dirty_layout_ |= internal::DirtyParagraph::RebuildLayout;
    }

    void Paragraph::erase_element(size_t i)
    {
        glyph_groups_.erase(glyph_groups_.begin() + i);
        dirty_layout_ |= internal::DirtyParagraph::RebuildLayout;
    }

    const Transform2D& Paragraph::get_local() const
    {
        return transformer_.get_local();
    }
    
    Transform2D& Paragraph::set_local()
    {
        return transformer_.set_local();
    }

    glm::vec2 Paragraph::get_content_size() const
    {
        return page_layout_.content_size;
    }
    
    glm::vec2 Paragraph::get_fitted_size() const
    {
        return page_layout_.fitted_size;
    }
    
    void Paragraph::draw() const
    {
        clean_dirty_layout();
        if (draw_bkg)
            bkg_.draw();
        for (const internal::GlyphGroup& glyph_group_ : glyph_groups_)
            glyph_group_.draw();
    }

    void Paragraph::init(std::vector<TextElement>&& elements)
    {
        bkg_.transformer.attach_parent(&transformer_);
        bkg_.transformer.set_modifier() = imp::make_poly<PivotTransformModifier2D>();
        bkg_.set_texture(graphics::textures::white1x1(), { 1.0f, 1.0f });
        bkg_.set_modulation({ 0.0f, 0.0f, 0.0f, 1.0f });

        for (TextElement& element : elements)
            add_element(std::move(element));
    }

    void Paragraph::clean_dirty_layout() const
    {
        if (dirty_layout_ & internal::DirtyParagraph::RebuildLayout)
        {
            build_layout();
            write_glyphs();

            auto& bkg_modifier = bkg_.transformer.ref_modifier<PivotTransformModifier2D>();
            bkg_modifier.size = page_layout_.fitted_size + 2.0f * format_.padding;
            bkg_modifier.pivot = format_.pivot;
            bkg_.set_local().scale = bkg_modifier.size;
        }
        else
        {
            AlignmentFlags flags = AlignmentFlags(0);
            const bool reposition_glyphs = dirty_layout_ != 0;
            bool resize_bkg = false;

            if (dirty_layout_ & internal::DirtyParagraph::LineSpacing)
            {
                flags = (AlignmentFlags)(flags | AlignmentFlags::Vertical);
                recompute_content_size_y(), recompute_fitted_size_y();
                resize_bkg = true;
            }

            if (dirty_layout_ & internal::DirtyParagraph::HorizontalAlign)
                flags = (AlignmentFlags)(flags | AlignmentFlags::Horizontal);

            if (dirty_layout_ & internal::DirtyParagraph::VerticalAlign)
                flags = (AlignmentFlags)(flags | AlignmentFlags::Vertical);

            if (dirty_layout_ & internal::DirtyParagraph::Padding)
            {
                flags = (AlignmentFlags)(flags | AlignmentFlags::Padding);
                resize_bkg = true;
            }

            if (dirty_layout_ & internal::DirtyParagraph::Pivot)
            {
                flags = (AlignmentFlags)(flags | AlignmentFlags::Pivot);
                bkg_.transformer.ref_modifier<PivotTransformModifier2D>().pivot = format_.pivot;
            }

            if (dirty_layout_ & internal::DirtyParagraph::MinSize)
            {
                flags = (AlignmentFlags)(flags | AlignmentFlags::Vertical | AlignmentFlags::Horizontal);
                recompute_fitted_size_x(), recompute_fitted_size_y();
                resize_bkg = true;
            }

            compute_alignment_cache(flags);
            if (reposition_glyphs)
                for (size_t i = 0; i < written_glyph_groups_; ++i)
                    glyph_groups_[i].reposition_glyphs();
            if (resize_bkg)
            {
                const glm::vec2 bkg_size = page_layout_.fitted_size + 2.0f * format_.padding;
                bkg_.transformer.ref_modifier<PivotTransformModifier2D>().size = bkg_size;
                bkg_.set_local().scale = bkg_size;
            }
        }

        dirty_layout_ = internal::DirtyParagraph(0);
    }

    void Paragraph::build_layout() const
    {
        dirty_layout_ = internal::DirtyParagraph(0);

        page_layout_ = {};
        page_data_ = {};
        page_data_.lines.push_back({});

        TypesetData typeset = {};
        for (size_t i = 0; i < glyph_groups_.size(); ++i)
            glyph_groups_[i].build_page_section(typeset, peek_next(i));
        page_data_.current_line().width = typeset.x;

        recompute_content_size_x(), recompute_content_size_y();
        recompute_fitted_size_x(), recompute_fitted_size_y();
        compute_alignment_cache(AlignmentFlags(~0));
    }

    void Paragraph::write_glyphs() const
    {
        TypesetData typeset = {};
        bool writing = true;
        written_glyph_groups_ = glyph_groups_.size();
        for (size_t i = 0; i < glyph_groups_.size(); ++i)
        {
            if (writing)
            {
                if (glyph_groups_[i].write_glyph_section(typeset, peek_next(i))
                    == internal::GlyphGroup::WriteResult::Break)
                {
                    written_glyph_groups_ = i + 1;
                    writing = false;
                }
            }
            else
                glyph_groups_[i].clear_cache();
        }
    }

    void Paragraph::compute_alignment_cache(AlignmentFlags flags) const
    {
        if (flags & AlignmentFlags::ResizeLines)
            alignment_cache_.lines.resize(page_data_.lines.size());

        if (flags & AlignmentFlags::Vertical)
        {
            for (size_t i = 0; i + 1 < alignment_cache_.lines.size(); ++i)
                alignment_cache_.lines[i].height = page_data_.lines[i].spaced_height(format_);
            
            if (!alignment_cache_.lines.empty())
                alignment_cache_.lines.back().height = page_data_.lines.back().max_height;

            alignment_cache_.valign_offset = 0.f;

            switch (format_.vertical_alignment)
            {
            case VerticalAlignment::Bottom:
                alignment_cache_.valign_offset = -(page_layout_.fitted_size.y - page_layout_.content_size.y);
                break;
            
            case VerticalAlignment::Middle:
                alignment_cache_.valign_offset = -0.5f * (page_layout_.fitted_size.y - page_layout_.content_size.y);
                break;
            
            case VerticalAlignment::Justify:
                if (page_data_.linebreaks > 0)
                {
                    const float extra_linebreak = (page_layout_.fitted_size.y - page_layout_.content_size.y)
                        / page_data_.linebreaks;

                    for (size_t i = 0; i < alignment_cache_.lines.size(); ++i)
                    {
                        if (page_data_.lines[i].width == 0.0f)
                            alignment_cache_.lines[i].height += extra_linebreak;
                    }
                }
                break;
            
            case VerticalAlignment::FullJustify:
                if (!page_data_.lines.empty()
                    && page_layout_.content_size.y - page_data_.lines.back().max_height > 0.0f)
                {
                    const float line_mult = (page_layout_.fitted_size.y - page_data_.lines.back().max_height)
                        / (page_layout_.content_size.y - page_data_.lines.back().max_height);

                    for (size_t i = 0; i < alignment_cache_.lines.size(); ++i)
                        alignment_cache_.lines[i].height *= line_mult;
                }
                break;
            }
        }

        if (flags & AlignmentFlags::Horizontal)
        {
            for (auto& line : alignment_cache_.lines)
            {
                line.start = 0.0f;
                line.space_width_mult = 1.0f;
                line.character_shift = 0.0f;
            }

            switch (format_.horizontal_alignment)
            {
            case HorizontalAlignment::Right:
                for (size_t i = 0; i < alignment_cache_.lines.size(); ++i)
                    alignment_cache_.lines[i].start = page_layout_.fitted_size.x - page_data_.lines[i].width;
                break;
            
            case HorizontalAlignment::Center:
                for (size_t i = 0; i < alignment_cache_.lines.size(); ++i)
                    alignment_cache_.lines[i].start = 0.5f * (page_layout_.fitted_size.x - page_data_.lines[i].width);
                break;
            
            case HorizontalAlignment::Justify:
                for (size_t i = 0; i < alignment_cache_.lines.size(); ++i)
                {
                    const internal::PageBuildData::Line line = page_data_.lines[i];
                    if (line.space_width > 0.0f)
                    {
                        alignment_cache_.lines[i].space_width_mult = 1.f
                            + (page_layout_.fitted_size.x - line.width) / line.space_width;
                    }
                }
                break;
            
            case HorizontalAlignment::FullJustify:
                for (size_t i = 0; i < alignment_cache_.lines.size(); ++i)
                {
                    const internal::PageBuildData::Line line = page_data_.lines[i];
                    alignment_cache_.lines[i].character_shift = line.characters > 1
                        ? (page_layout_.fitted_size.x - line.width) / (line.characters - 1)
                        : 0.f;
                }
                break;
            }
        }

        if (flags & AlignmentFlags::Pivot)
            alignment_cache_.pivot_offset = page_layout_.fitted_size * (glm::vec2{ 0.0f, 1.0f } - format_.pivot);

        if (flags & AlignmentFlags::Padding)
            alignment_cache_.padding_offset = format_.padding * glm::vec2{ 1.0f, -1.0f };
    }

    internal::GlyphGroup::PeekData Paragraph::peek_next(size_t i) const
    {
        if (i + 1 >= glyph_groups_.size())
            return {};

        const internal::GlyphGroup& current_glyph = glyph_groups_[i];
        const internal::GlyphGroup& next_glyph = glyph_groups_[i + 1];

        if (current_glyph.element_.font.adj_compat(next_glyph.element_.font))
            return next_glyph.peek();
        else
            return {};
    }

    void Paragraph::recompute_content_size_x() const
    {
        page_layout_.content_size.x = 0.0f;
        for (size_t i = 0; i < page_data_.lines.size(); ++i)
            page_layout_.content_size.x = glm::max(page_layout_.content_size.x, page_data_.lines[i].width);
    }

    void Paragraph::recompute_content_size_y() const
    {
        page_layout_.content_size.y = 0.0f;
        for (size_t i = 0; i < page_data_.lines.size(); ++i)
        {
            if (i + 1 < page_data_.lines.size())
                page_layout_.content_size.y += page_data_.lines[i].spaced_height(format_);
            else
                page_layout_.content_size.y += page_data_.lines[i].max_height;
        }
    }

    void Paragraph::recompute_fitted_size_x() const
    {
        page_layout_.fitted_size.x = glm::max(page_layout_.content_size.x, format_.min_size.x);
    }

    void Paragraph::recompute_fitted_size_y() const
    {
        page_layout_.fitted_size.y = glm::max(page_layout_.content_size.y, format_.min_size.y);
    }

    static void add_text_element(TOMLNode element, size_t i, std::vector<TextElement>& elements)
    {
        try
        {
            TextElement e;
            assets::Parser parser(element, { "in text element #", i });
            const auto font = parser.required<std::string>(detail::Key::Font)();
            const auto font_index = parser.defaulted(detail::Key::FontIndex)(0u);
            e.font = context::load_font(font, font_index);
            e.text = parser.required<std::string>(detail::Key::Text)();

            parser.optional(detail::Key::TextColor)(e.text_color);
            parser.optional(detail::Key::AdjacentOffset)(e.adj_offset);
            parser.optional(detail::Key::Scale)(e.scale);
            if (auto line_y_pivot = parser.optional<float>(detail::Key::LineYPivot)())
                e.line_y_pivot = *line_y_pivot;
            parser.optional(detail::Key::JitterOffset)(e.jitter_offset);

            if (parser.defaulted(detail::Key::Expand)(false))
                TextElement::expand(e, elements);
            else
                elements.push_back(std::move(e));
        }
        catch (const Error& e)
        {
            if (e.code != ErrorCode::LoadAsset)
                throw;
        }
    }

    Paragraph Paragraph::load(TOMLNode node)
    {
        assets::Parser parser(node);

        std::vector<TextElement> elements;
        if (auto element_array = parser.optional<TOMLArray, true>(detail::Key::Element)())
        {
            for (size_t i = 0; i < element_array->size(); ++i)
                if (auto element = TOMLNode(*element_array->get(i)))
                    add_text_element(element, i, elements);
        }
        else if (auto element = parser.optional<TOMLNode>(detail::Key::Element)())
            add_text_element(*element, 0, elements);

        Paragraph paragraph(std::move(elements), ParagraphFormat::load(parser.field(detail::Key::Format)));
        if (auto transformer = parser.optional<TOMLNode>(detail::Key::Transformer)())
        {
            paragraph.set_local() = Transform2D::load(*transformer);
            paragraph.set_transformer().set_modifier() = TransformModifier2D::load(*transformer);
        }

        parser.optional(detail::Key::DrawBackground)(paragraph.draw_bkg);
        if (auto bkg_color = parser.optional<glm::vec4>(detail::Key::BackgroundColor)())
            paragraph.set_bkg_color(*bkg_color);

        paragraph.set_camera_invariant(parser.defaulted(detail::Key::CameraInvariant)(false));

        return paragraph;
    }

    Paragraph Paragraph::load(TOMLNode node, const DebugTrace& trace)
    {
        auto scope = trace.scope("ASSETS", "oly::rendering::Paragraph::load()");
        return load(node);
    }
}
