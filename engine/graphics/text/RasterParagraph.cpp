#include "RasterParagraph.h"

#include "core/base/Errors.h"
#include "core/context/rendering/Fonts.h"
#include "core/context/rendering/Sprites.h"
#include "core/context/rendering/Textures.h"
#include "graphics/resources/Textures.h"

namespace oly::rendering
{
	RasterParagraphFormatExposure::RasterParagraphFormatExposure(RasterParagraph& paragraph)
		: paragraph(paragraph)
	{
	}

	void RasterParagraphFormatExposure::set_line_spacing(float line_spacing)
	{
		if (paragraph.format.line_spacing != line_spacing)
		{
			paragraph.format.line_spacing = line_spacing;
			if (paragraph.format.max_height > 0.0f)
				paragraph.dirty_layout |= internal::DirtyParagraph::REBUILD_LAYOUT;
			else
				paragraph.dirty_layout |= internal::DirtyParagraph::LINE_SPACING;
		}
	}

	void RasterParagraphFormatExposure::set_tab_spaces(float tab_spaces)
	{
		if (paragraph.format.tab_spaces != tab_spaces)
		{
			paragraph.format.tab_spaces = tab_spaces;
			paragraph.dirty_layout |= internal::DirtyParagraph::REBUILD_LAYOUT;
		}
	}

	void RasterParagraphFormatExposure::set_linebreak_spacing(float linebreak_spacing)
	{
		if (paragraph.format.linebreak_spacing != linebreak_spacing)
		{
			paragraph.format.linebreak_spacing = linebreak_spacing;
			if (paragraph.format.max_height > 0.0f)
				paragraph.dirty_layout |= internal::DirtyParagraph::REBUILD_LAYOUT;
			else
				paragraph.dirty_layout |= internal::DirtyParagraph::LINE_SPACING;
		}
	}

	void RasterParagraphFormatExposure::set_pivot(glm::vec2 pivot)
	{
		if (paragraph.format.pivot != pivot)
		{
			paragraph.format.pivot = pivot;
			paragraph.dirty_layout |= internal::DirtyParagraph::PIVOT;
		}
	}

	void RasterParagraphFormatExposure::set_min_size(glm::vec2 min_size)
	{
		if (paragraph.format.min_size != min_size)
		{
			paragraph.format.min_size = min_size;
			paragraph.dirty_layout |= internal::DirtyParagraph::MIN_SIZE;
		}
	}

	void RasterParagraphFormatExposure::set_text_wrap(float text_wrap)
	{
		if (paragraph.format.text_wrap != text_wrap)
		{
			paragraph.format.text_wrap = text_wrap;
			paragraph.dirty_layout |= internal::DirtyParagraph::REBUILD_LAYOUT;
		}
	}

	void RasterParagraphFormatExposure::set_max_height(float max_height)
	{
		if (paragraph.format.max_height != max_height)
		{
			paragraph.format.max_height = max_height;
			paragraph.dirty_layout |= internal::DirtyParagraph::REBUILD_LAYOUT;
		}
	}

	void RasterParagraphFormatExposure::set_padding(glm::vec2 padding)
	{
		if (paragraph.format.padding != padding)
		{
			paragraph.format.padding = padding;
			paragraph.dirty_layout |= internal::DirtyParagraph::PADDING;
		}
	}

	void RasterParagraphFormatExposure::set_horizontal_alignment(ParagraphFormat::HorizontalAlignment alignment)
	{
		if (paragraph.format.horizontal_alignment != alignment)
		{
			paragraph.format.horizontal_alignment = alignment;
			paragraph.dirty_layout |= internal::DirtyParagraph::HORIZONTAL_ALIGN;
		}
	}

	void RasterParagraphFormatExposure::set_vertical_alignment(ParagraphFormat::VerticalAlignment alignment)
	{
		if (paragraph.format.vertical_alignment != alignment)
		{
			paragraph.format.vertical_alignment = alignment;
			paragraph.dirty_layout |= internal::DirtyParagraph::VERTICAL_ALIGN;
		}
	}

	internal::RasterGlyphGroup::RasterGlyphGroup(RasterTextElement&& element)
		: element(std::move(element))
	{
	}

	void internal::RasterGlyphGroup::set_batch(Unbatched)
	{
		for (auto& glyph : glyphs)
			glyph.set_batch(UNBATCHED);
	}

	void internal::RasterGlyphGroup::set_batch(rendering::SpriteBatch& batch)
	{
		for (auto& glyph : glyphs)
			glyph.set_batch(batch);
	}

	void internal::RasterGlyphGroup::draw() const
	{
		if (dirty & DirtyGlyphGroup::LINE_ALIGNMENT)
			realign_lines();
		if (dirty & DirtyGlyphGroup::JITTER_OFFSET)
			reposition_jitter();
		if (dirty & DirtyGlyphGroup::RECOLOR)
			recolor();

		for (const TextGlyph& glyph : glyphs)
			glyph.draw();
	}

	internal::RasterGlyphGroup::PeekData internal::RasterGlyphGroup::peek() const
	{
		auto iter = element.base.text.begin();
		return { .first_codepoint = iter ? iter.codepoint() : utf::Codepoint(0) };
	}

	void internal::RasterGlyphGroup::build_page_section(TypesetData& typeset, PeekData next_peek) const
	{
		auto iter = element.base.text.begin();
		if (!iter)
			return;

		paragraph->page_data.current_line().max_height = glm::max(paragraph->page_data.current_line().max_height, element.line_height());
		build_adj_offset(typeset, next_peek);

		while (iter)
		{
			utf::Codepoint codepoint = iter.advance();
			utf::Codepoint next_codepoint = iter ? iter.codepoint() : next_peek.first_codepoint;

			if (codepoint == ' ')
				build_space(typeset, next_codepoint);
			else if (codepoint == '\t')
				build_tab(typeset, next_codepoint);
			else if (utf::is_n_or_r(codepoint))
			{
				if (iter && utf::is_rn(codepoint, next_codepoint))
					++iter;
				build_newline(typeset);
				if (iter.codepoint()) // next codepoint in group
					paragraph->page_data.current_line().max_height = element.line_height();
			}
			else if (element.font->supports(codepoint))
			{
				float dx = advance_width(codepoint, next_codepoint);
				if (!can_fit_on_line(typeset, dx))
					build_newline(typeset);
				build_glyph(typeset, dx);
			}
			else
			{
				OLY_LOG_WARNING(true, "RENDERING") << LOG.source_info.full_source() << "Font does not support the glyph with codepoint (" << codepoint << ")." << LOG.nl;
			}
		}
	}

	internal::RasterGlyphGroup::WriteResult internal::RasterGlyphGroup::write_glyph_section(TypesetData& typeset, PeekData next_peek) const
	{
		dirty = internal::DirtyGlyphGroup(0);
		clear_cache();

		auto iter = element.base.text.begin();
		if (!iter)
			return WriteResult::CONTINUE;

		LineAlignment line{ .y_offset = element.base.line_y_pivot * (paragraph->page_data.lines[typeset.line].max_height - element.line_height()) };

		if (!write_adj_offset(typeset, next_peek, line))
			return WriteResult::BREAK;

		while (iter)
		{
			utf::Codepoint codepoint = iter.advance();
			utf::Codepoint next_codepoint = iter ? iter.codepoint() : next_peek.first_codepoint;

			if (codepoint == ' ')
				write_space(typeset, next_codepoint);
			else if (codepoint == '\t')
				write_tab(typeset, next_codepoint);
			else if (utf::is_n_or_r(codepoint))
			{
				if (iter && utf::is_rn(codepoint, next_codepoint))
					++iter;
				if (!write_newline(typeset, line))
					return WriteResult::BREAK;
			}
			else if (element.font->supports(codepoint))
			{
				float dx = advance_width(codepoint, next_codepoint);
				if (!can_fit_on_line(typeset, dx))
				{
					if (!write_newline(typeset, line))
						return WriteResult::BREAK;
				}
				write_glyph(typeset, codepoint, dx, line);
			}
			else
			{
				OLY_LOG_WARNING(true, "RENDERING") << LOG.source_info.full_source() << "Font does not support the glyph with codepoint (" << codepoint << ")." << LOG.nl;
			}
		}

		recolor();
		return WriteResult::CONTINUE;
	}

	void internal::RasterGlyphGroup::clear_cache() const
	{
		glyphs.clear();
		cached_info.clear();
	}

	bool internal::RasterGlyphGroup::can_fit_on_line(const TypesetData& typeset, float dx) const
	{
		return paragraph->format.text_wrap <= 0.0f || typeset.x + dx <= paragraph->format.text_wrap;
	}

	bool internal::RasterGlyphGroup::can_fit_vertically(const TypesetData& typeset, float dy) const
	{
		return paragraph->format.max_height <= 0.0f || -typeset.y + dy <= paragraph->format.max_height;
	}

	void internal::RasterGlyphGroup::build_adj_offset(TypesetData& typeset, PeekData next_peek) const
	{
		if (element.base.adj_offset <= 0.0f || typeset.x == 0.0f)
			return;

		auto iter = element.base.text.begin();
		const utf::Codepoint codepoint = iter.advance();
		if (utf::is_n_or_r(codepoint))
			return;

		const utf::Codepoint next_codepoint = iter ? iter.codepoint() : next_peek.first_codepoint;
		float dx = 0.0f;
		if (codepoint == ' ')
			dx = space_width(next_codepoint);
		else if (codepoint == '\t')
			dx = tab_width(next_codepoint);
		else if (element.font->supports(codepoint))
			dx = advance_width(codepoint, next_codepoint);
		else
		{
			OLY_LOG_WARNING(true, "RENDERING") << LOG.source_info.full_source() << "Font does not support the glyph with codepoint (" << codepoint << ")." << LOG.nl;
		}

		if (can_fit_on_line(typeset, element.base.adj_offset + dx))
		{
			typeset.x += element.base.adj_offset;
			paragraph->page_data.current_line().space_width += element.base.adj_offset;
			++paragraph->page_data.current_line().characters;
		}
		else
		{
			build_newline(typeset);
			if (iter.codepoint()) // next codepoint in group
				paragraph->page_data.current_line().max_height = element.line_height();
		}
	}

	void internal::RasterGlyphGroup::build_space(TypesetData& typeset, utf::Codepoint next_codepoint) const
	{
		const float dx = space_width(next_codepoint);
		typeset.x += dx;
		paragraph->page_data.current_line().space_width += dx;
		++paragraph->page_data.current_line().characters;
	}

	void internal::RasterGlyphGroup::build_tab(TypesetData& typeset, utf::Codepoint next_codepoint) const
	{
		const float dx = tab_width(next_codepoint);
		typeset.x += dx;
		paragraph->page_data.current_line().space_width += dx;
		++paragraph->page_data.current_line().characters;
	}

	void internal::RasterGlyphGroup::build_newline(TypesetData& typeset) const
	{
		if (typeset.x == 0.0f)
			++paragraph->page_data.linebreaks;

		paragraph->page_data.current_line().width = typeset.x;
		++typeset.line;
		typeset.y -= paragraph->page_data.current_line().spaced_height(paragraph->format);
		typeset.x = 0.0f;
		paragraph->page_data.lines.emplace_back();
	}

	void internal::RasterGlyphGroup::build_glyph(TypesetData& typeset, float dx) const
	{
		typeset.x += dx;
		paragraph->page_data.current_line().final_advance_width = dx;
		++paragraph->page_data.current_line().characters;
	}

	bool internal::RasterGlyphGroup::write_adj_offset(TypesetData& typeset, PeekData next_peek, LineAlignment& line) const
	{
		if (element.base.adj_offset <= 0.0f || typeset.x == 0.0f)
			return true;

		auto iter = element.base.text.begin();
		const utf::Codepoint codepoint = iter.advance();
		if (utf::is_n_or_r(codepoint))
			return true;

		const utf::Codepoint next_codepoint = iter ? iter.codepoint() : next_peek.first_codepoint;
		float dx = 0.0f;
		if (codepoint == ' ')
			dx = space_width(next_codepoint);
		else if (codepoint == '\t')
			dx = tab_width(next_codepoint);
		else if (element.font->supports(codepoint))
			dx = advance_width(codepoint, next_codepoint);
		else
		{
			OLY_LOG_WARNING(true, "RENDERING") << LOG.source_info.full_source() << "Font does not support the glyph with codepoint (" << codepoint << ")." << LOG.nl;
		}

		if (can_fit_on_line(typeset, element.base.adj_offset + dx))
		{
			typeset.x += element.base.adj_offset * paragraph->alignment_cache.lines[typeset.line].space_width_mult;
			++typeset.character;
			return true;
		}
		else
			return write_newline(typeset, line);
	}

	void internal::RasterGlyphGroup::write_space(TypesetData& typeset, utf::Codepoint next_codepoint) const
	{
		typeset.x += space_width(next_codepoint) * paragraph->alignment_cache.lines[typeset.line].space_width_mult;
		++typeset.character;
	}

	void internal::RasterGlyphGroup::write_tab(TypesetData& typeset, utf::Codepoint next_codepoint) const
	{
		typeset.x += tab_width(next_codepoint) * paragraph->alignment_cache.lines[typeset.line].space_width_mult;
		++typeset.character;
	}

	bool internal::RasterGlyphGroup::write_newline(TypesetData& typeset, LineAlignment& line) const
	{
		const float dy = paragraph->alignment_cache.lines[typeset.line].height;
		if (!can_fit_vertically(typeset, dy))
			return false;

		++typeset.line;
		typeset.y -= dy;
		typeset.x = 0.0f;
		typeset.character = 0;

		if (typeset.line < paragraph->alignment_cache.lines.size())
			line.y_offset = element.base.line_y_pivot * (paragraph->page_data.lines[typeset.line].max_height - element.line_height());

		return true;
	}

	void internal::RasterGlyphGroup::write_glyph(TypesetData& typeset, utf::Codepoint c, float dx, LineAlignment line) const
	{
		cached_info.push_back(CachedGlyphInfo{ .typeset = typeset, .line_y_offset = line.y_offset });
		TextGlyph glyph;
		glyph.transformer.attach_parent(&paragraph->transformer);
		glyph.set_glyph(*element.font, element.font->get_glyph(c), get_glyph_position(glyphs.size()), element.base.scale);
		glyphs.push_back(std::move(glyph));
		typeset.x += dx;
		++typeset.character;
	}

	glm::vec2 internal::RasterGlyphGroup::get_glyph_position(size_t i) const
	{
		const CachedGlyphInfo& cache = cached_info[i];
		return paragraph->alignment_cache.position(cache.typeset) + glm::vec2{ 0.0f, cache.line_y_offset } + element.base.jitter_offset;
	}

	float internal::RasterGlyphGroup::space_width(utf::Codepoint next_codepoint) const
	{
		float adv = element.font->get_space_advance_width();
		if (next_codepoint)
			adv += element.font->kerning_of(next_codepoint, utf::Codepoint(' '));
		return adv * element.base.scale.x;
	}

	float internal::RasterGlyphGroup::tab_width(utf::Codepoint next_codepoint) const
	{
		return space_width(next_codepoint) * paragraph->format.tab_spaces * element.base.scale.x;
	}

	float internal::RasterGlyphGroup::advance_width(utf::Codepoint codepoint, utf::Codepoint next_codepoint) const
	{
		const RasterFontGlyph& font_glyph = element.font->get_glyph(codepoint);
		float adv = font_glyph.advance_width * element.font->get_scale().x;
		if (next_codepoint)
			adv += element.font->kerning_of(codepoint, next_codepoint);
		return adv * element.base.scale.x;
	}

	void internal::RasterGlyphGroup::recolor() const
	{
		dirty &= ~DirtyGlyphGroup::RECOLOR;
		for (TextGlyph& glyph : glyphs)
			glyph.set_text_color(element.base.text_color);
	}

	void internal::RasterGlyphGroup::realign_lines() const
	{
		dirty &= ~DirtyGlyphGroup::LINE_ALIGNMENT;

		float new_line_y_offset = 0.0f;
		size_t last_line = -1;

		for (size_t i = 0; i < glyphs.size(); ++i)
		{
			CachedGlyphInfo& info = cached_info[i];

			if (info.typeset.line != last_line)
				new_line_y_offset = element.base.line_y_pivot * (paragraph->page_data.lines[info.typeset.line].max_height - element.line_height());

			if (info.line_y_offset != new_line_y_offset)
			{
				TextGlyph& glyph = glyphs[i];
				glyph.set_local().position.y -= info.line_y_offset;
				info.line_y_offset = new_line_y_offset;
				glyph.set_local().position.y += info.line_y_offset;
			}
		}
	}

	void internal::RasterGlyphGroup::reposition_jitter() const
	{
		dirty &= ~DirtyGlyphGroup::JITTER_OFFSET;
		for (TextGlyph& glyph : glyphs)
			glyph.set_local().position += element.base.jitter_offset - last_jitter_offset;
	}

	void internal::RasterGlyphGroup::reposition_glyphs() const
	{
		for (size_t i = 0; i < glyphs.size(); ++i)
			glyphs[i].set_local().position = get_glyph_position(i);
	}

	RasterTextElementExposure::RasterTextElementExposure(RasterParagraph& paragraph, internal::RasterGlyphGroup& glyph_group)
		: paragraph(paragraph), glyph_group(glyph_group)
	{
	}

	void RasterTextElementExposure::set_font(const RasterFontRef& font)
	{
		if (glyph_group.element.font != font)
		{
			glyph_group.element.font = font;
			paragraph.dirty_layout |= internal::DirtyParagraph::REBUILD_LAYOUT;
		}
	}

	void RasterTextElementExposure::set_text(utf::String&& text)
	{
		if (glyph_group.element.base.text != text)
		{
			glyph_group.element.base.text = std::move(text);
			paragraph.dirty_layout |= internal::DirtyParagraph::REBUILD_LAYOUT;
		}
	}

	void RasterTextElementExposure::set_text_color(glm::vec4 color)
	{
		if (glyph_group.element.base.text_color != color)
		{
			glyph_group.element.base.text_color = color;
			glyph_group.dirty |= internal::DirtyGlyphGroup::RECOLOR;
		}
	}

	void RasterTextElementExposure::set_adj_offset(float adj_offset)
	{
		if (glyph_group.element.base.adj_offset != adj_offset)
		{
			glyph_group.element.base.adj_offset = adj_offset;
			paragraph.dirty_layout |= internal::DirtyParagraph::REBUILD_LAYOUT;
		}
	}

	void RasterTextElementExposure::set_scale(glm::vec2 scale)
	{
		if (glyph_group.element.base.scale != scale)
		{
			glyph_group.element.base.scale = scale;
			paragraph.dirty_layout |= internal::DirtyParagraph::REBUILD_LAYOUT;
		}
	}

	void RasterTextElementExposure::set_line_y_pivot(float line_y_pivot)
	{
		if (glyph_group.element.base.line_y_pivot != line_y_pivot)
		{
			glyph_group.element.base.line_y_pivot = line_y_pivot;
			glyph_group.dirty |= internal::DirtyGlyphGroup::LINE_ALIGNMENT;
		}
	}

	void RasterTextElementExposure::set_jitter_offset(glm::vec2 jitter_offset)
	{
		if (glyph_group.dirty & internal::DirtyGlyphGroup::JITTER_OFFSET)
			glyph_group.element.base.jitter_offset = jitter_offset;
		else if (glyph_group.element.base.jitter_offset != jitter_offset)
		{
			glyph_group.dirty |= internal::DirtyGlyphGroup::JITTER_OFFSET;
			glyph_group.last_jitter_offset = glyph_group.element.base.jitter_offset;
			glyph_group.element.base.jitter_offset = jitter_offset;
		}
	}

	RasterParagraph::RasterParagraph(std::vector<RasterTextElement>&& elements, const ParagraphFormat& format)
		: format(format), bkg()
	{
		init(std::move(elements));
	}

	RasterParagraph::RasterParagraph(Unbatched, std::vector<RasterTextElement>&& elements, const ParagraphFormat& format)
		: format(format), bkg(UNBATCHED)
	{
		init(std::move(elements));
	}

	RasterParagraph::RasterParagraph(SpriteBatch& batch, std::vector<RasterTextElement>&& elements, const ParagraphFormat& format)
		: format(format), bkg(batch)
	{
		init(std::move(elements));
	}

	RasterParagraph::RasterParagraph(const RasterParagraph& other)
		: bkg(other.bkg), format(other.format), dirty_layout(other.dirty_layout), glyph_groups(other.glyph_groups),
		page_data(other.page_data), page_layout(other.page_layout), transformer(other.transformer), draw_bkg(other.draw_bkg)
	{
		bkg.transformer.attach_parent(&transformer);

		for (internal::RasterGlyphGroup& glyph_group : glyph_groups)
			glyph_group.paragraph = this;
	}

	RasterParagraph::RasterParagraph(RasterParagraph&& other) noexcept
		: bkg(std::move(other.bkg)), format(std::move(other.format)), dirty_layout(other.dirty_layout), glyph_groups(std::move(other.glyph_groups)),
		page_data(std::move(other.page_data)), page_layout(std::move(other.page_layout)), transformer(std::move(other.transformer)), draw_bkg(other.draw_bkg)
	{
		for (internal::RasterGlyphGroup& glyph_group : glyph_groups)
			glyph_group.paragraph = this;
	}

	RasterParagraph& RasterParagraph::operator=(const RasterParagraph& other)
	{
		if (this != &other)
			*this = dupl(other);
		return *this;
	}

	RasterParagraph& RasterParagraph::operator=(RasterParagraph&& other) noexcept
	{
		if (this != &other)
		{
			bkg = std::move(other.bkg);
			format = std::move(other.format);
			dirty_layout = other.dirty_layout;
			glyph_groups = std::move(other.glyph_groups);
			page_data = std::move(other.page_data);
			page_layout = std::move(other.page_layout);
			transformer = std::move(other.transformer);
			draw_bkg = other.draw_bkg;

			for (internal::RasterGlyphGroup& glyph_group : glyph_groups)
				glyph_group.paragraph = this;
		}
		return *this;
	}

	void RasterParagraph::init(std::vector<RasterTextElement>&& elements)
	{
		bkg.transformer.attach_parent(&transformer);
		bkg.transformer.set_modifier() = std::make_unique<PivotTransformModifier2D>();
		bkg.set_texture(graphics::textures::white1x1, { 1.0f, 1.0f });
		bkg.set_modulation({ 0.0f, 0.0f, 0.0f, 1.0f });

		for (RasterTextElement& element : elements)
			add_element(std::move(element));
	}

	void RasterParagraph::set_batch(Unbatched)
	{
		bkg.set_batch(UNBATCHED);
		for (auto& glyph_group : glyph_groups)
			glyph_group.set_batch(UNBATCHED);
	}

	void RasterParagraph::set_batch(SpriteBatch& batch)
	{
		bkg.set_batch(batch);
		for (auto& glyph_group : glyph_groups)
			glyph_group.set_batch(batch);
	}

	glm::vec4 RasterParagraph::get_bkg_color() const
	{
		return bkg.get_modulation();
	}

	void RasterParagraph::set_bkg_color(glm::vec4 color)
	{
		bkg.set_modulation(color);
	}

	size_t RasterParagraph::get_element_count() const
	{
		return glyph_groups.size();
	}

	void RasterParagraph::add_element(RasterTextElement&& element)
	{
		glyph_groups.emplace_back(std::move(element));
		glyph_groups.back().paragraph = this;
		dirty_layout |= internal::DirtyParagraph::REBUILD_LAYOUT;
	}

	void RasterParagraph::insert_element(size_t i, RasterTextElement&& element)
	{
		if (i > glyph_groups.size())
			throw Error(ErrorCode::INDEX_OUT_OF_RANGE);

		glyph_groups.emplace(glyph_groups.begin() + i, std::move(element));
		glyph_groups[i].paragraph = this;
		dirty_layout |= internal::DirtyParagraph::REBUILD_LAYOUT;
	}

	void RasterParagraph::erase_element(size_t i)
	{
		glyph_groups.erase(glyph_groups.begin() + i);
		dirty_layout |= internal::DirtyParagraph::REBUILD_LAYOUT;
	}

	void RasterParagraph::draw() const
	{
		clean_dirty_layout();
		if (draw_bkg)
			bkg.draw();
		for (const internal::RasterGlyphGroup& glyph_group : glyph_groups)
			glyph_group.draw();
	}

	void RasterParagraph::clean_dirty_layout() const
	{
		if (dirty_layout & internal::DirtyParagraph::REBUILD_LAYOUT)
		{
			build_layout();
			write_glyphs();

			auto& bkg_modifier = bkg.transformer.ref_modifier<PivotTransformModifier2D>();
			bkg_modifier.size = page_layout.fitted_size + 2.0f * format.padding;
			bkg_modifier.pivot = format.pivot;
			bkg.set_local().scale = bkg_modifier.size;
		}
		else
		{
			AlignmentFlags flags = AlignmentFlags(0);
			const bool reposition_glyphs = dirty_layout != 0;
			bool resize_bkg = false;

			if (dirty_layout & internal::DirtyParagraph::LINE_SPACING)
			{
				flags = (AlignmentFlags)(flags | AlignmentFlags::VERTICAL);
				recompute_content_size_y(), recompute_fitted_size_y();
				resize_bkg = true;
			}

			if (dirty_layout & internal::DirtyParagraph::HORIZONTAL_ALIGN)
				flags = (AlignmentFlags)(flags | AlignmentFlags::HORIZONTAL);

			if (dirty_layout & internal::DirtyParagraph::VERTICAL_ALIGN)
				flags = (AlignmentFlags)(flags | AlignmentFlags::VERTICAL);

			if (dirty_layout & internal::DirtyParagraph::PADDING)
			{
				flags = (AlignmentFlags)(flags | AlignmentFlags::PADDING);
				resize_bkg = true;
			}

			if (dirty_layout & internal::DirtyParagraph::PIVOT)
			{
				flags = (AlignmentFlags)(flags | AlignmentFlags::PIVOT);
				bkg.transformer.ref_modifier<PivotTransformModifier2D>().pivot = format.pivot;
			}

			if (dirty_layout & internal::DirtyParagraph::MIN_SIZE)
			{
				flags = (AlignmentFlags)(flags | AlignmentFlags::VERTICAL | AlignmentFlags::HORIZONTAL);
				recompute_fitted_size_x(), recompute_fitted_size_y();
				resize_bkg = true;
			}

			compute_alignment_cache(flags);
			if (reposition_glyphs)
				for (size_t i = 0; i < written_glyph_groups; ++i)
					glyph_groups[i].reposition_glyphs();
			if (resize_bkg)
			{
				const glm::vec2 bkg_size = page_layout.fitted_size + 2.0f * format.padding;
				bkg.transformer.ref_modifier<PivotTransformModifier2D>().size = bkg_size;
				bkg.set_local().scale = bkg_size;
			}
		}

		dirty_layout = internal::DirtyParagraph(0);
	}

	void RasterParagraph::build_layout() const
	{
		dirty_layout = internal::DirtyParagraph(0);

		page_layout = {};
		page_data = {};
		page_data.lines.push_back({});

		internal::TypesetData typeset = {};
		for (size_t i = 0; i < glyph_groups.size(); ++i)
			glyph_groups[i].build_page_section(typeset, peek_next(i));
		page_data.current_line().width = typeset.x;

		recompute_content_size_x(), recompute_content_size_y();
		recompute_fitted_size_x(), recompute_fitted_size_y();
		compute_alignment_cache(AlignmentFlags(~0));
	}

	void RasterParagraph::write_glyphs() const
	{
		internal::TypesetData typeset = {};
		bool writing = true;
		written_glyph_groups = glyph_groups.size();
		for (size_t i = 0; i < glyph_groups.size(); ++i)
		{
			if (writing)
			{
				if (glyph_groups[i].write_glyph_section(typeset, peek_next(i)) == internal::RasterGlyphGroup::WriteResult::BREAK)
				{
					written_glyph_groups = i + 1;
					writing = false;
				}
			}
			else
				glyph_groups[i].clear_cache();
		}
	}

	void RasterParagraph::compute_alignment_cache(AlignmentFlags flags) const
	{
		if (flags & AlignmentFlags::RESIZE_LINES)
			alignment_cache.lines.resize(page_data.lines.size());

		if (flags & AlignmentFlags::VERTICAL)
		{
			for (size_t i = 0; i + 1 < alignment_cache.lines.size(); ++i)
				alignment_cache.lines[i].height = page_data.lines[i].spaced_height(format);
			if (!alignment_cache.lines.empty())
				alignment_cache.lines.back().height = page_data.lines.back().max_height;

			alignment_cache.valign_offset = 0.0f;
			switch (format.vertical_alignment)
			{
			case ParagraphFormat::VerticalAlignment::BOTTOM:
				alignment_cache.valign_offset = -(page_layout.fitted_size.y - page_layout.content_size.y);
				break;
			case ParagraphFormat::VerticalAlignment::MIDDLE:
				alignment_cache.valign_offset = -0.5f * (page_layout.fitted_size.y - page_layout.content_size.y);
				break;
			case ParagraphFormat::VerticalAlignment::JUSTIFY:
				if (page_data.linebreaks > 0)
				{
					const float extra_linebreak = (page_layout.fitted_size.y - page_layout.content_size.y) / page_data.linebreaks;
					for (size_t i = 0; i < alignment_cache.lines.size(); ++i)
						if (page_data.lines[i].width == 0.0f)
							alignment_cache.lines[i].height += extra_linebreak;
				}
				break;
			case ParagraphFormat::VerticalAlignment::FULL_JUSTIFY:
				if (!page_data.lines.empty() && page_layout.content_size.y - page_data.lines.back().max_height > 0.0f)
				{
					const float line_mult = (page_layout.fitted_size.y - page_data.lines.back().max_height) / (page_layout.content_size.y - page_data.lines.back().max_height);
					for (size_t i = 0; i < alignment_cache.lines.size(); ++i)
						alignment_cache.lines[i].height *= line_mult;
				}
				break;
			}
		}

		if (flags & AlignmentFlags::HORIZONTAL)
		{
			for (auto& line : alignment_cache.lines)
			{
				line.start = 0.0f;
				line.space_width_mult = 1.0f;
				line.character_shift = 0.0f;
			}

			switch (format.horizontal_alignment)
			{
			case ParagraphFormat::HorizontalAlignment::RIGHT:
				for (size_t i = 0; i < alignment_cache.lines.size(); ++i)
					alignment_cache.lines[i].start = page_layout.fitted_size.x - page_data.lines[i].width;
				break;
			case ParagraphFormat::HorizontalAlignment::CENTER:
				for (size_t i = 0; i < alignment_cache.lines.size(); ++i)
					alignment_cache.lines[i].start = 0.5f * (page_layout.fitted_size.x - page_data.lines[i].width);
				break;
			case ParagraphFormat::HorizontalAlignment::JUSTIFY:
				for (size_t i = 0; i < alignment_cache.lines.size(); ++i)
				{
					const internal::PageBuildData::Line line = page_data.lines[i];
					if (line.space_width > 0.0f)
						alignment_cache.lines[i].space_width_mult = 1.0f + (page_layout.fitted_size.x - line.width) / line.space_width;
				}
				break;
			case ParagraphFormat::HorizontalAlignment::FULL_JUSTIFY:
				for (size_t i = 0; i < alignment_cache.lines.size(); ++i)
				{
					const internal::PageBuildData::Line line = page_data.lines[i];
					alignment_cache.lines[i].character_shift = line.characters > 1 ? (page_layout.fitted_size.x - line.width) / (line.characters - 1) : 0.0f;
				}
				break;
			}
		}

		if (flags & AlignmentFlags::PIVOT)
			alignment_cache.pivot_offset = page_layout.fitted_size * (glm::vec2{ 0.0f, 1.0f } - format.pivot);

		if (flags & AlignmentFlags::PADDING)
			alignment_cache.padding_offset = format.padding * glm::vec2{ 1.0f, -1.0f };
	}

	internal::RasterGlyphGroup::PeekData RasterParagraph::peek_next(size_t i) const
	{
		if (i + 1 >= glyph_groups.size())
			return {};

		const internal::RasterGlyphGroup& current_glyph = glyph_groups[i];
		const internal::RasterGlyphGroup& next_glyph = glyph_groups[i + 1];

		if (current_glyph.element.font == next_glyph.element.font)
			return next_glyph.peek();
		else
			return {};
	}

	void RasterParagraph::recompute_content_size_x() const
	{
		page_layout.content_size.x = 0.0f;
		for (size_t i = 0; i < page_data.lines.size(); ++i)
			page_layout.content_size.x = glm::max(page_layout.content_size.x, page_data.lines[i].width);
	}

	void RasterParagraph::recompute_content_size_y() const
	{
		page_layout.content_size.y = 0.0f;
		for (size_t i = 0; i < page_data.lines.size(); ++i)
		{
			if (i + 1 < page_data.lines.size())
				page_layout.content_size.y += page_data.lines[i].spaced_height(format);
			else
				page_layout.content_size.y += page_data.lines[i].max_height;
		}
	}

	void RasterParagraph::recompute_fitted_size_x() const
	{
		page_layout.fitted_size.x = glm::max(page_layout.content_size.x, format.min_size.x);
	}

	void RasterParagraph::recompute_fitted_size_y() const
	{
		page_layout.fitted_size.y = glm::max(page_layout.content_size.y, format.min_size.y);
	}
}
