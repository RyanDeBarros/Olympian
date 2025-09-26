#include "Paragraph.h"

#include "core/base/Errors.h"
#include "core/context/rendering/Fonts.h"
#include "core/context/rendering/Sprites.h"
#include "core/context/rendering/Textures.h"
#include "graphics/resources/Textures.h"

namespace oly::rendering
{
	ParagraphFormatExposure::ParagraphFormatExposure(Paragraph& paragraph)
		: paragraph(paragraph)
	{
	}
	
	void ParagraphFormatExposure::set_line_spacing(float line_spacing)
	{
		paragraph.format.line_spacing = line_spacing;
		paragraph.dirty_layout |= internal::DirtyParagraph::REBUILD_LAYOUT;
	}
	
	void ParagraphFormatExposure::set_tab_spaces(float tab_spaces)
	{
		paragraph.format.tab_spaces = tab_spaces;
		paragraph.dirty_layout |= internal::DirtyParagraph::REBUILD_LAYOUT;
	}
	
	void ParagraphFormatExposure::set_linebreak_spacing(float linebreak_spacing)
	{
		paragraph.format.linebreak_spacing = linebreak_spacing;
		paragraph.dirty_layout |= internal::DirtyParagraph::REBUILD_LAYOUT;
	}
	
	void ParagraphFormatExposure::set_pivot(glm::vec2 pivot)
	{
		paragraph.format.pivot = pivot;
		paragraph.dirty_layout |= internal::DirtyParagraph::REBUILD_LAYOUT;
	}
	
	void ParagraphFormatExposure::set_min_size(glm::vec2 min_size)
	{
		paragraph.format.min_size = min_size;
		paragraph.dirty_layout |= internal::DirtyParagraph::REBUILD_LAYOUT;
	}
	
	void ParagraphFormatExposure::set_text_wrap(float text_wrap)
	{
		paragraph.format.text_wrap = text_wrap;
		paragraph.dirty_layout |= internal::DirtyParagraph::REBUILD_LAYOUT;
	}
	
	void ParagraphFormatExposure::set_max_height(float max_height)
	{
		paragraph.format.max_height = max_height;
		paragraph.dirty_layout |= internal::DirtyParagraph::REBUILD_LAYOUT;
	}

	void ParagraphFormatExposure::set_padding(glm::vec2 padding)
	{
		paragraph.format.padding = padding;
		paragraph.dirty_layout |= internal::DirtyParagraph::REBUILD_LAYOUT; // TODO v5 PADDING flag
	}
	
	void ParagraphFormatExposure::set_horizontal_alignment(ParagraphFormat::HorizontalAlignment alignment)
	{
		paragraph.format.horizontal_alignment = alignment;
		paragraph.dirty_layout |= internal::DirtyParagraph::REBUILD_LAYOUT; // TODO v5 HORIZONTAL_ALIGNMENT flag
	}
	
	void ParagraphFormatExposure::set_vertical_alignment(ParagraphFormat::VerticalAlignment alignment)
	{
		paragraph.format.vertical_alignment = alignment;
		paragraph.dirty_layout |= internal::DirtyParagraph::REBUILD_LAYOUT; // TODO v5 VERTICAL_ALIGNMENT flag
	}

	internal::GlyphGroup::GlyphGroup(TextElement&& element)
		: element(std::move(element))
	{
	}

	void internal::GlyphGroup::set_batch(SpriteBatch* batch)
	{
		for (auto& glyph : glyphs)
			glyph.set_batch(batch);
	}

	void internal::GlyphGroup::draw() const
	{
		if (dirty & DirtyGlyphGroup::LINE_ALIGNMENT)
			realign_line();
		if (dirty & DirtyGlyphGroup::RECOLOR)
			recolor();

		for (const TextGlyph& glyph : glyphs)
			glyph.draw();
	}
	
	internal::GlyphGroup::PeekData internal::GlyphGroup::peek() const
	{
		auto iter = element.text.begin();
		return { .first_codepoint = iter ? iter.codepoint() : utf::Codepoint(0) };
	}

	void internal::GlyphGroup::build_page_section(TypesetData& typeset, PeekData next_peek) const
	{
		auto iter = element.text.begin();
		if (!iter)
			return;

		paragraph->pagedata.current_line().max_height = glm::max(paragraph->pagedata.current_line().max_height, element.line_height());
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
					paragraph->pagedata.current_line().max_height = element.line_height();
			}
			else if (element.font->cache(codepoint))
			{
				float dx = advance_width(codepoint, next_codepoint);
				if (!can_fit_on_line(typeset, dx))
					build_newline(typeset);
				build_glyph(typeset, dx);
			}
		}
	}

	internal::GlyphGroup::WriteResult internal::GlyphGroup::write_glyph_section(TypesetData& typeset, PeekData next_peek, const AlignmentCache& alignment) const
	{
		dirty &= ~DirtyGlyphGroup::LINE_ALIGNMENT;
		glyphs.clear();
		cached_info.clear();

		auto iter = element.text.begin();
		if (!iter)
			return WriteResult::CONTINUE;

		LineAlignment line{ .y_offset = element.line_y_pivot * (paragraph->pagedata.lines[typeset.line].max_height - element.line_height()) };

		if (!write_adj_offset(typeset, next_peek, alignment, line))
			return WriteResult::BREAK;

		while (iter)
		{
			utf::Codepoint codepoint = iter.advance();
			utf::Codepoint next_codepoint = iter ? iter.codepoint() : next_peek.first_codepoint;

			if (codepoint == ' ')
				write_space(typeset, next_codepoint, alignment);
			else if (codepoint == '\t')
				write_tab(typeset, next_codepoint, alignment);
			else if (utf::is_n_or_r(codepoint))
			{
				if (iter && utf::is_rn(codepoint, next_codepoint))
					++iter;
				if (!write_newline(typeset, alignment, line))
					return WriteResult::BREAK;
			}
			else if (element.font->cache(codepoint))
			{
				float dx = advance_width(codepoint, next_codepoint);
				if (!can_fit_on_line(typeset, dx))
				{
					if (!write_newline(typeset, alignment, line))
						return WriteResult::BREAK;
				}
				write_glyph(typeset, codepoint, dx, alignment, line);
			}
		}

		recolor();
		return WriteResult::CONTINUE;
	}

	bool internal::GlyphGroup::can_fit_on_line(const TypesetData& typeset, float dx) const
	{
		return paragraph->format.text_wrap <= 0.0f || typeset.x + dx <= paragraph->format.text_wrap;
	}

	bool internal::GlyphGroup::can_fit_vertically(const TypesetData& typeset, float dy) const
	{
		return paragraph->format.max_height <= 0.0f || -typeset.y + dy <= paragraph->format.max_height;
	}

	void internal::GlyphGroup::build_adj_offset(TypesetData& typeset, PeekData next_peek) const
	{
		if (element.adj_offset <= 0.0f || typeset.x == 0.0f)
			return;

		auto iter = element.text.begin();
		const utf::Codepoint codepoint = iter.advance();
		if (utf::is_n_or_r(codepoint))
			return;

		const utf::Codepoint next_codepoint = iter ? iter.codepoint() : next_peek.first_codepoint;
		float dx = 0.0f;
		if (codepoint == ' ')
			dx = space_width(next_codepoint);
		else if (codepoint == '\t')
			dx = tab_width(next_codepoint);
		else if (element.font->cache(codepoint))
			dx = advance_width(codepoint, next_codepoint);

		if (can_fit_on_line(typeset, element.adj_offset + dx))
		{
			typeset.x += element.adj_offset;
			paragraph->pagedata.current_line().space_width += element.adj_offset;
			++paragraph->pagedata.current_line().characters;
		}
		else
		{
			build_newline(typeset);
			if (iter.codepoint()) // next codepoint in group
				paragraph->pagedata.current_line().max_height = element.line_height();
		}
	}

	void internal::GlyphGroup::build_space(TypesetData& typeset, utf::Codepoint next_codepoint) const
	{
		const float dx = space_width(next_codepoint);
		typeset.x += dx;
		paragraph->pagedata.current_line().space_width += dx;
		++paragraph->pagedata.current_line().characters;
	}

	void internal::GlyphGroup::build_tab(TypesetData& typeset, utf::Codepoint next_codepoint) const
	{
		const float dx = tab_width(next_codepoint);
		typeset.x += dx;
		paragraph->pagedata.current_line().space_width += dx;
		++paragraph->pagedata.current_line().characters;
	}

	void internal::GlyphGroup::build_newline(TypesetData& typeset) const
	{
		if (typeset.x == 0.0f)
			++paragraph->pagedata.linebreaks;

		paragraph->pagedata.current_line().width = typeset.x;
		++typeset.line;
		typeset.y -= paragraph->pagedata.current_line().spaced_height(paragraph->format);
		typeset.x = 0.0f;
		paragraph->pagedata.lines.emplace_back();
	}

	void internal::GlyphGroup::build_glyph(TypesetData& typeset, float dx) const
	{
		typeset.x += dx;
		paragraph->pagedata.current_line().final_advance_width = dx;
		++paragraph->pagedata.current_line().characters;
	}

	bool internal::GlyphGroup::write_adj_offset(TypesetData& typeset, PeekData next_peek, const AlignmentCache& alignment, LineAlignment& line) const
	{
		if (element.adj_offset <= 0.0f || typeset.x == 0.0f)
			return true;

		auto iter = element.text.begin();
		const utf::Codepoint codepoint = iter.advance();
		if (utf::is_n_or_r(codepoint))
			return true;

		const utf::Codepoint next_codepoint = iter ? iter.codepoint() : next_peek.first_codepoint;
		float dx = 0.0f;
		if (codepoint == ' ')
			dx = space_width(next_codepoint);
		else if (codepoint == '\t')
			dx = tab_width(next_codepoint);
		else if (element.font->cache(codepoint))
			dx = advance_width(codepoint, next_codepoint);

		if (can_fit_on_line(typeset, element.adj_offset + dx))
		{
			typeset.x += element.adj_offset * alignment.lines[typeset.line].space_width_mult;
			++typeset.character;
			return true;
		}
		else
			return write_newline(typeset, alignment, line);
	}

	void internal::GlyphGroup::write_space(TypesetData& typeset, utf::Codepoint next_codepoint, const AlignmentCache& alignment) const
	{
		typeset.x += space_width(next_codepoint) * alignment.lines[typeset.line].space_width_mult;
		++typeset.character;
	}

	void internal::GlyphGroup::write_tab(TypesetData& typeset, utf::Codepoint next_codepoint, const AlignmentCache& alignment) const
	{
		typeset.x += tab_width(next_codepoint) * alignment.lines[typeset.line].space_width_mult;
		++typeset.character;
	}

	bool internal::GlyphGroup::write_newline(TypesetData& typeset, const AlignmentCache& alignment, LineAlignment& line) const
	{
		const float dy = alignment.lines[typeset.line].height;
		if (!can_fit_vertically(typeset, dy))
			return false;

		++typeset.line;
		typeset.y -= dy;
		typeset.x = 0.0f;
		typeset.character = 0;

		if (typeset.line < alignment.lines.size())
			line.y_offset = element.line_y_pivot * (paragraph->pagedata.lines[typeset.line].max_height - element.line_height());

		return true;
	}

	void internal::GlyphGroup::write_glyph(TypesetData& typeset, utf::Codepoint c, float dx, const AlignmentCache& alignment, LineAlignment line) const
	{
		TextGlyph glyph;
		glyph.transformer.attach_parent(&paragraph->transformer);
		glyph.set_glyph(*element.font, element.font->get_glyph(c), alignment.position(typeset) + glm::vec2{ 0.0f, line.y_offset }, element.scale);
		glyphs.push_back(std::move(glyph));
		cached_info.push_back(CachedGlyphInfo{ .line_y_offset = line.y_offset, .line = typeset.line });
		typeset.x += dx;
		++typeset.character;
	}

	float internal::GlyphGroup::space_width(utf::Codepoint next_codepoint) const
	{
		float adv = element.font->get_space_advance_width();
		if (next_codepoint)
			adv += element.font->kerning_of(next_codepoint, utf::Codepoint(' '));
		return adv * element.scale.x;
	}

	float internal::GlyphGroup::tab_width(utf::Codepoint next_codepoint) const
	{
		return space_width(next_codepoint) * paragraph->format.tab_spaces * element.scale.x;
	}

	float internal::GlyphGroup::advance_width(utf::Codepoint codepoint, utf::Codepoint next_codepoint) const
	{
		const FontGlyph& font_glyph = element.font->get_glyph(codepoint);
		float adv = font_glyph.advance_width * element.font->get_scale();
		if (next_codepoint)
			adv += element.font->kerning_of(codepoint, next_codepoint, font_glyph.index, element.font->get_glyph_index(next_codepoint));
		return adv * element.scale.x;
	}

	void internal::GlyphGroup::recolor() const
	{
		dirty &= ~DirtyGlyphGroup::RECOLOR;
		for (TextGlyph& glyph : glyphs)
			glyph.set_text_color(element.text_color);
	}

	void internal::GlyphGroup::realign_line() const
	{
		dirty &= ~DirtyGlyphGroup::LINE_ALIGNMENT;

		float new_line_y_offset = 0.0f;
		size_t last_line = -1;

		for (size_t i = 0; i < glyphs.size(); ++i)
		{
			CachedGlyphInfo& info = cached_info[i];

			if (info.line != last_line)
				new_line_y_offset = element.line_y_pivot * (paragraph->pagedata.lines[info.line].max_height - element.line_height());

			if (info.line_y_offset != new_line_y_offset)
			{
				TextGlyph& glyph = glyphs[i];
				glyph.set_local().position.y -= info.line_y_offset;
				info.line_y_offset = new_line_y_offset;
				glyph.set_local().position.y += info.line_y_offset;
			}
		}
	}

	TextElementExposure::TextElementExposure(Paragraph& paragraph, internal::GlyphGroup& glyph_group)
		: paragraph(paragraph), glyph_group(glyph_group)
	{
	}

	void TextElementExposure::set_font(const FontAtlasRef& font)
	{
		glyph_group.element.font = font;
		paragraph.dirty_layout |= internal::DirtyParagraph::REBUILD_LAYOUT;
	}
	
	void TextElementExposure::set_text(utf::String&& text)
	{
		glyph_group.element.text = std::move(text);
		paragraph.dirty_layout |= internal::DirtyParagraph::REBUILD_LAYOUT;
	}
	
	void TextElementExposure::set_text_color(glm::vec4 color)
	{
		glyph_group.element.text_color = color;
		glyph_group.dirty |= internal::DirtyGlyphGroup::RECOLOR;
	}
	
	void TextElementExposure::set_adj_offset(float adj_offset)
	{
		glyph_group.element.adj_offset = adj_offset;
		paragraph.dirty_layout |= internal::DirtyParagraph::REBUILD_LAYOUT;
	}
	
	void TextElementExposure::set_scale(glm::vec2 scale)
	{
		glyph_group.element.scale = scale;
		paragraph.dirty_layout |= internal::DirtyParagraph::REBUILD_LAYOUT;
	}
	
	void TextElementExposure::set_line_y_pivot(float line_y_pivot)
	{
		glyph_group.element.line_y_pivot = line_y_pivot;
		glyph_group.dirty |= internal::DirtyGlyphGroup::LINE_ALIGNMENT;
	}

	Paragraph::Paragraph(std::vector<TextElement>&& elements, const ParagraphFormat& format)
		: format(format), bkg()
	{
		init(std::move(elements));
	}

	Paragraph::Paragraph(SpriteBatch* batch, std::vector<TextElement>&& elements, const ParagraphFormat& format)
		: format(format), bkg(batch)
	{
		init(std::move(elements));
	}

	Paragraph::Paragraph(const Paragraph& other)
		: bkg(other.bkg), format(other.format), dirty_layout(other.dirty_layout), glyph_groups(other.glyph_groups),
		pagedata(other.pagedata), page_layout(other.page_layout), transformer(other.transformer), draw_bkg(other.draw_bkg)
	{
		bkg.transformer.attach_parent(&transformer);
		
		for (internal::GlyphGroup& glyph_group : glyph_groups)
			glyph_group.paragraph = this;
	}

	Paragraph::Paragraph(Paragraph&& other) noexcept
		: bkg(std::move(other.bkg)), format(std::move(other.format)), dirty_layout(other.dirty_layout), glyph_groups(std::move(other.glyph_groups)),
		pagedata(std::move(other.pagedata)), page_layout(std::move(other.page_layout)), transformer(std::move(other.transformer)), draw_bkg(other.draw_bkg)
	{
		for (internal::GlyphGroup& glyph_group : glyph_groups)
			glyph_group.paragraph = this;
	}

	Paragraph& Paragraph::operator=(const Paragraph& other)
	{
		if (this != &other)
			*this = dupl(other);
		return *this;
	}

	Paragraph& Paragraph::operator=(Paragraph&& other) noexcept
	{
		if (this != &other)
		{
			bkg = std::move(other.bkg);
			format = std::move(other.format);
			dirty_layout = other.dirty_layout;
			glyph_groups = std::move(other.glyph_groups);
			pagedata = std::move(other.pagedata);
			page_layout = std::move(other.page_layout);
			transformer = std::move(other.transformer);
			draw_bkg = other.draw_bkg;

			for (internal::GlyphGroup& glyph_group : glyph_groups)
				glyph_group.paragraph = this;
		}
		return *this;
	}

	void Paragraph::init(std::vector<TextElement>&& elements)
	{
		bkg.transformer.attach_parent(&transformer);
		bkg.transformer.set_modifier() = std::make_unique<PivotTransformModifier2D>();
		bkg.set_texture(graphics::textures::white1x1, { 1.0f, 1.0f });
		bkg.set_modulation({ 0.0f, 0.0f, 0.0f, 1.0f });

		for (TextElement& element : elements)
			add_element(std::move(element));
	}

	void Paragraph::set_batch(SpriteBatch* batch)
	{
		bkg.set_batch(batch);
		for (auto& glyph_group : glyph_groups)
			glyph_group.set_batch(batch);
	}

	glm::vec4 Paragraph::get_bkg_color() const
	{
		return bkg.get_modulation();
	}

	void Paragraph::set_bkg_color(glm::vec4 color)
	{
		bkg.set_modulation(color);
	}

	size_t Paragraph::get_element_count() const
	{
		return glyph_groups.size();
	}
	
	void Paragraph::add_element(TextElement&& element)
	{
		glyph_groups.emplace_back(std::move(element));
		glyph_groups.back().paragraph = this;
		dirty_layout |= internal::DirtyParagraph::REBUILD_LAYOUT;
	}
	
	void Paragraph::insert_element(size_t i, TextElement&& element)
	{
		if (i > glyph_groups.size())
			throw Error(ErrorCode::INDEX_OUT_OF_RANGE);

		glyph_groups.emplace(glyph_groups.begin() + i, std::move(element));
		glyph_groups[i].paragraph = this;
		dirty_layout |= internal::DirtyParagraph::REBUILD_LAYOUT;
	}
	
	void Paragraph::erase_element(size_t i)
	{
		glyph_groups.erase(glyph_groups.begin() + i);
		dirty_layout |= internal::DirtyParagraph::REBUILD_LAYOUT;
	}

	void Paragraph::draw() const
	{
		if (dirty_layout & internal::DirtyParagraph::REBUILD_LAYOUT)
		{
			build_layout();
			dirty_layout = internal::DirtyParagraph(0);
		}
		else
		{
			
		}

		if (draw_bkg)
			bkg.draw();
		for (const internal::GlyphGroup& glyph_group : glyph_groups)
			glyph_group.draw();
	}

	void Paragraph::build_layout() const
	{
		build_page();
		write_glyphs();

		auto& bkg_modifier = bkg.transformer.ref_modifier<PivotTransformModifier2D>();
		bkg_modifier.size = page_layout.fitted_size + 2.0f * format.padding;
		bkg_modifier.pivot = format.pivot;
		bkg.set_local().scale = bkg_modifier.size;
	}

	void Paragraph::build_page() const
	{
		page_layout = {};
		pagedata = {};
		pagedata.lines.push_back({});

		internal::TypesetData typeset = {};
		for (size_t i = 0; i < glyph_groups.size(); ++i)
			glyph_groups[i].build_page_section(typeset, peek_next(i));
		pagedata.current_line().width = typeset.x;

		for (size_t i = 0; i < pagedata.lines.size(); ++i)
		{
			const internal::PageBuildData::Line line = pagedata.lines[i];
			page_layout.content_size.x = glm::max(page_layout.content_size.x, line.width);
			if (i + 1 < pagedata.lines.size())
				page_layout.content_size.y += line.spaced_height(format);
			else
				page_layout.content_size.y += line.max_height;
		}

		page_layout.fitted_size = { glm::max(page_layout.content_size.x, format.min_size.x), glm::max(page_layout.content_size.y, format.min_size.y) };
	}

	void Paragraph::write_glyphs() const
	{
		internal::TypesetData typeset = {};
		for (size_t i = 0; i < glyph_groups.size(); ++i)
		{
			internal::GlyphGroup::WriteResult result = glyph_groups[i].write_glyph_section(typeset, peek_next(i), get_alignment_cache());
			if (result == internal::GlyphGroup::WriteResult::BREAK)
				break;
		}
	}

	internal::AlignmentCache Paragraph::get_alignment_cache() const
	{
		internal::AlignmentCache alignment_cache{};
		alignment_cache.lines.resize(pagedata.lines.size());

		// line height
		for (size_t i = 0; i + 1 < alignment_cache.lines.size(); ++i)
			alignment_cache.lines[i].height = pagedata.lines[i].spaced_height(format);
		if (!alignment_cache.lines.empty())
			alignment_cache.lines.back().height = pagedata.lines.back().max_height;

		// vertical alignment
		switch (format.vertical_alignment)
		{
		case ParagraphFormat::VerticalAlignment::BOTTOM:
			alignment_cache.valign_offset = -(page_layout.fitted_size.y - page_layout.content_size.y);
			break;
		case ParagraphFormat::VerticalAlignment::MIDDLE:
			alignment_cache.valign_offset = -0.5f * (page_layout.fitted_size.y - page_layout.content_size.y);
			break;
		case ParagraphFormat::VerticalAlignment::JUSTIFY:
			if (pagedata.linebreaks > 0)
			{
				const float extra_linebreak = (page_layout.fitted_size.y - page_layout.content_size.y) / pagedata.linebreaks;
				for (size_t i = 0; i < alignment_cache.lines.size(); ++i)
				{
					if (pagedata.lines[i].width == 0.0f)
						alignment_cache.lines[i].height += extra_linebreak;
				}
			}
			break;
		case ParagraphFormat::VerticalAlignment::FULL_JUSTIFY:
			if (!pagedata.lines.empty() && page_layout.content_size.y - pagedata.lines.back().max_height > 0.0f)
			{
				const float line_mult = (page_layout.fitted_size.y - pagedata.lines.back().max_height) / (page_layout.content_size.y - pagedata.lines.back().max_height);
				for (size_t i = 0; i < alignment_cache.lines.size(); ++i)
					alignment_cache.lines[i].height *= line_mult;
			}
			break;
		}

		// horizontal alignment
		switch (format.horizontal_alignment)
		{
		case ParagraphFormat::HorizontalAlignment::RIGHT:
			for (size_t i = 0; i < alignment_cache.lines.size(); ++i)
				alignment_cache.lines[i].start = page_layout.fitted_size.x - pagedata.lines[i].width;
			break;
		case ParagraphFormat::HorizontalAlignment::CENTER:
			for (size_t i = 0; i < alignment_cache.lines.size(); ++i)
				alignment_cache.lines[i].start = 0.5f * (page_layout.fitted_size.x - pagedata.lines[i].width);
			break;
		case ParagraphFormat::HorizontalAlignment::JUSTIFY:
			for (size_t i = 0; i < alignment_cache.lines.size(); ++i)
			{
				const internal::PageBuildData::Line line = pagedata.lines[i];
				if (line.space_width > 0.0f)
					alignment_cache.lines[i].space_width_mult = 1.0f + (page_layout.fitted_size.x - line.width) / line.space_width;
			}
			break;
		case ParagraphFormat::HorizontalAlignment::FULL_JUSTIFY:
			for (size_t i = 0; i < alignment_cache.lines.size(); ++i)
			{
				const internal::PageBuildData::Line line = pagedata.lines[i];
				alignment_cache.lines[i].character_shift = line.characters > 1 ? (page_layout.fitted_size.x - line.width) / (line.characters - 1) : 0.0f;
			}
			break;
		}

		// page offsets
		alignment_cache.pivot_offset = page_layout.fitted_size * (glm::vec2{ 0.0f, 1.0f } - format.pivot);
		alignment_cache.padding_offset = format.padding * glm::vec2{ 1.0f, -1.0f };

		return alignment_cache;
	}

	internal::GlyphGroup::PeekData Paragraph::peek_next(size_t i) const
	{
		if (i + 1 >= glyph_groups.size())
			return {};

		const internal::GlyphGroup& current_glyph = glyph_groups[i];
		const internal::GlyphGroup& next_glyph = glyph_groups[i + 1];

		if (current_glyph.element.font->font_face() == next_glyph.element.font->font_face())
			return next_glyph.peek();
		else
			return {};
	}
}
