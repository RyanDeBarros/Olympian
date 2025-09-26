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
		paragraph.dirty_layout |= internal::DirtyParagraph::REBUILD_LAYOUT;
	}
	
	void ParagraphFormatExposure::set_horizontal_alignment(ParagraphFormat::HorizontalAlignment alignment)
	{
		paragraph.format.horizontal_alignment = alignment;
		paragraph.dirty_layout |= internal::DirtyParagraph::REBUILD_LAYOUT;
	}
	
	void ParagraphFormatExposure::set_vertical_alignment(ParagraphFormat::VerticalAlignment alignment)
	{
		paragraph.format.vertical_alignment = alignment;
		paragraph.dirty_layout |= internal::DirtyParagraph::REBUILD_LAYOUT;
	}

	Paragraph::GlyphGroup::GlyphGroup(TextElement&& element)
		: element(std::move(element))
	{
	}

	void Paragraph::GlyphGroup::set_batch(SpriteBatch* batch)
	{
		for (auto& glyph : glyphs)
			glyph.set_batch(batch);
	}

	void Paragraph::GlyphGroup::draw() const
	{
		for (const TextGlyph& glyph : glyphs)
			glyph.draw();
	}
	
	Paragraph::GlyphGroup::PeekData Paragraph::GlyphGroup::peek() const
	{
		auto iter = element.text.begin();
		return { .first_codepoint = iter ? iter.codepoint() : utf::Codepoint(0) };
	}

	void Paragraph::GlyphGroup::build_page_section(const Paragraph& paragraph, PageBuildData& pagedata, TypesetData& typeset, PeekData next_peek) const
	{
		auto iter = element.text.begin();
		if (!iter)
			return;

		pagedata.current_line().max_height = glm::max(pagedata.current_line().max_height, element.line_height());
		build_adj_offset(pagedata, paragraph.format, typeset, next_peek);

		while (iter)
		{
			utf::Codepoint codepoint = iter.advance();
			utf::Codepoint next_codepoint = iter ? iter.codepoint() : next_peek.first_codepoint;

			if (codepoint == ' ')
				build_space(pagedata, typeset, next_codepoint);
			else if (codepoint == '\t')
				build_tab(pagedata, paragraph.format, typeset, next_codepoint);
			else if (utf::is_n_or_r(codepoint))
			{
				if (iter && utf::is_rn(codepoint, next_codepoint))
					++iter;
				build_newline(pagedata, paragraph.format, typeset);
				if (iter.codepoint()) // next codepoint in group
					pagedata.current_line().max_height = element.line_height();
			}
			else if (element.font->cache(codepoint))
			{
				float dx = advance_width(codepoint, next_codepoint);
				if (!can_fit_on_line(paragraph.format, typeset, dx))
					build_newline(pagedata, paragraph.format, typeset);
				build_glyph(pagedata, typeset, dx);
			}
		}
	}

	Paragraph::GlyphGroup::WriteResult Paragraph::GlyphGroup::write_glyph_section(const Paragraph& paragraph,
		const PageBuildData& pagedata, TypesetData& typeset, PeekData next_peek, const AlignmentCache& alignment) const
	{
		glyphs.clear();

		auto iter = element.text.begin();
		if (!iter)
			return WriteResult::CONTINUE;

		LineAlignment line{ .y_offset = element.line_y_pivot * (pagedata.lines[typeset.line].max_height - element.line_height()) };

		if (!write_adj_offset(pagedata, paragraph.format, typeset, next_peek, alignment, line))
			return WriteResult::BREAK;

		while (iter)
		{
			utf::Codepoint codepoint = iter.advance();
			utf::Codepoint next_codepoint = iter ? iter.codepoint() : next_peek.first_codepoint;

			if (codepoint == ' ')
				write_space(typeset, next_codepoint, alignment);
			else if (codepoint == '\t')
				write_tab(paragraph.format, typeset, next_codepoint, alignment);
			else if (utf::is_n_or_r(codepoint))
			{
				if (iter && utf::is_rn(codepoint, next_codepoint))
					++iter;
				if (!write_newline(pagedata, paragraph.format, typeset, alignment, line))
					return WriteResult::BREAK;
			}
			else if (element.font->cache(codepoint))
			{
				float dx = advance_width(codepoint, next_codepoint);
				if (!can_fit_on_line(paragraph.format, typeset, dx))
				{
					if (!write_newline(pagedata, paragraph.format, typeset, alignment, line))
						return WriteResult::BREAK;
				}
				write_glyph(paragraph, typeset, codepoint, dx, alignment, line);
			}
		}

		set_glyph_attributes(); // TODO v5 if changing text color, no need to re-build/write entire section - just call set_glyph_attributes. use separate dirty flag
		return WriteResult::CONTINUE;
	}

	bool Paragraph::GlyphGroup::can_fit_on_line(const ParagraphFormat& format, const TypesetData& typeset, float dx) const
	{
		return format.text_wrap <= 0.0f || typeset.x + dx <= format.text_wrap;
	}

	bool Paragraph::GlyphGroup::can_fit_vertically(const ParagraphFormat& format, const TypesetData& typeset, float dy) const
	{
		return format.max_height <= 0.0f || -typeset.y + dy <= format.max_height;
	}

	void Paragraph::GlyphGroup::build_adj_offset(PageBuildData& pagedata, const ParagraphFormat& format, TypesetData& typeset, PeekData next_peek) const
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
			dx = tab_width(format, next_codepoint);
		else if (element.font->cache(codepoint))
			dx = advance_width(codepoint, next_codepoint);

		if (can_fit_on_line(format, typeset, element.adj_offset + dx))
		{
			typeset.x += element.adj_offset;
			pagedata.current_line().space_width += element.adj_offset;
			++pagedata.current_line().characters;
		}
		else
		{
			build_newline(pagedata, format, typeset);
			if (iter.codepoint()) // next codepoint in group
				pagedata.current_line().max_height = element.line_height();
		}
	}

	void Paragraph::GlyphGroup::build_space(PageBuildData& pagedata, TypesetData& typeset, utf::Codepoint next_codepoint) const
	{
		const float dx = space_width(next_codepoint);
		typeset.x += dx;
		pagedata.current_line().space_width += dx;
		++pagedata.current_line().characters;
	}

	void Paragraph::GlyphGroup::build_tab(PageBuildData& pagedata, const ParagraphFormat& format, TypesetData& typeset, utf::Codepoint next_codepoint) const
	{
		const float dx = tab_width(format, next_codepoint);
		typeset.x += dx;
		pagedata.current_line().space_width += dx;
		++pagedata.current_line().characters;
	}

	void Paragraph::GlyphGroup::build_newline(PageBuildData& pagedata, const ParagraphFormat& format, TypesetData& typeset) const
	{
		if (typeset.x == 0.0f)
			++pagedata.linebreaks;

		pagedata.current_line().width = typeset.x;
		++typeset.line;
		typeset.y -= pagedata.current_line().spaced_height(format);
		typeset.x = 0.0f;
		pagedata.lines.emplace_back();
	}

	void Paragraph::GlyphGroup::build_glyph(PageBuildData& pagedata, TypesetData& typeset, float dx) const
	{
		typeset.x += dx;
		pagedata.current_line().final_advance_width = dx;
		++pagedata.current_line().characters;
	}

	bool Paragraph::GlyphGroup::write_adj_offset(const PageBuildData& pagedata, const ParagraphFormat& format,
		TypesetData& typeset, PeekData next_peek, const AlignmentCache& alignment, LineAlignment& line) const
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
			dx = tab_width(format, next_codepoint);
		else if (element.font->cache(codepoint))
			dx = advance_width(codepoint, next_codepoint);

		if (can_fit_on_line(format, typeset, element.adj_offset + dx))
		{
			typeset.x += element.adj_offset * alignment.lines[typeset.line].space_width_mult;
			++typeset.character;
			return true;
		}
		else
			return write_newline(pagedata, format, typeset, alignment, line);
	}

	void Paragraph::GlyphGroup::write_space(TypesetData& typeset, utf::Codepoint next_codepoint, const AlignmentCache& alignment) const
	{
		typeset.x += space_width(next_codepoint) * alignment.lines[typeset.line].space_width_mult;
		++typeset.character;
	}

	void Paragraph::GlyphGroup::write_tab(const ParagraphFormat& format, TypesetData& typeset, utf::Codepoint next_codepoint, const AlignmentCache& alignment) const
	{
		typeset.x += tab_width(format, next_codepoint) * alignment.lines[typeset.line].space_width_mult;
		++typeset.character;
	}

	bool Paragraph::GlyphGroup::write_newline(const PageBuildData& pagedata, const ParagraphFormat& format, TypesetData& typeset, const AlignmentCache& alignment, LineAlignment& line) const
	{
		const float dy = alignment.lines[typeset.line].height;
		if (!can_fit_vertically(format, typeset, dy))
			return false;

		++typeset.line;
		typeset.y -= dy;
		typeset.x = 0.0f;
		typeset.character = 0;

		if (typeset.line < alignment.lines.size())
			line.y_offset = element.line_y_pivot * (pagedata.lines[typeset.line].max_height - element.line_height());

		return true;
	}

	void Paragraph::GlyphGroup::write_glyph(const Paragraph& paragraph, TypesetData& typeset, utf::Codepoint c, float dx, const AlignmentCache& alignment, LineAlignment line) const
	{
		TextGlyph glyph = create_glyph(paragraph);
		glyph.set_glyph(*element.font, element.font->get_glyph(c), alignment.position(typeset) + glm::vec2{ 0.0f, line.y_offset }, element.scale);
		glyphs.emplace_back(std::move(glyph));
		typeset.x += dx;
		++typeset.character;
	}

	float Paragraph::GlyphGroup::space_width(utf::Codepoint next_codepoint) const
	{
		float adv = element.font->get_space_advance_width();
		if (next_codepoint)
			adv += element.font->kerning_of(next_codepoint, utf::Codepoint(' '));
		return adv * element.scale.x;
	}

	float Paragraph::GlyphGroup::tab_width(const ParagraphFormat& format, utf::Codepoint next_codepoint) const
	{
		return space_width(next_codepoint) * format.tab_spaces * element.scale.x;
	}

	float Paragraph::GlyphGroup::advance_width(utf::Codepoint codepoint, utf::Codepoint next_codepoint) const
	{
		const FontGlyph& font_glyph = element.font->get_glyph(codepoint);
		float adv = font_glyph.advance_width * element.font->get_scale();
		if (next_codepoint)
			adv += element.font->kerning_of(codepoint, next_codepoint, font_glyph.index, element.font->get_glyph_index(next_codepoint));
		return adv * element.scale.x;
	}

	TextGlyph Paragraph::GlyphGroup::create_glyph(const Paragraph& paragraph) const
	{
		TextGlyph glyph;
		glyph.transformer.attach_parent(&paragraph._transformer);
		return glyph;
	}
	
	void Paragraph::GlyphGroup::set_glyph_attributes() const
	{
		for (TextGlyph& glyph : glyphs)
			glyph.set_text_color(element.text_color);
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
	
	void Paragraph::init(std::vector<TextElement>&& elements)
	{
		for (TextElement& element : elements)
			add_element(std::move(element));

		bkg.transformer.attach_parent(&_transformer);
		bkg.transformer.set_modifier() = std::make_unique<PivotTransformModifier2D>();
		bkg.set_texture(graphics::textures::white1x1, { 1.0f, 1.0f });
		bkg.set_modulation({ 0.0f, 0.0f, 0.0f, 1.0f });
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

	const TextElement& Paragraph::get_element(size_t i) const
	{
		return glyph_groups[i].element;
	}

	TextElement& Paragraph::set_element(size_t i)
	{
		dirty_layout |= internal::DirtyParagraph::REBUILD_LAYOUT;
		return glyph_groups[i].element;
	}

	size_t Paragraph::get_element_count() const
	{
		return glyph_groups.size();
	}
	
	void Paragraph::add_element(TextElement&& element)
	{
		glyph_groups.emplace_back(std::move(element));
		dirty_layout |= internal::DirtyParagraph::REBUILD_LAYOUT;
	}
	
	void Paragraph::insert_element(size_t i, TextElement&& element)
	{
		glyph_groups.emplace(glyph_groups.begin() + i, std::move(element));
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
		for (const GlyphGroup& glyph_group : glyph_groups)
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

		TypesetData typeset = {};
		for (size_t i = 0; i < glyph_groups.size(); ++i)
			glyph_groups[i].build_page_section(*this, pagedata, typeset, peek_next(i));
		pagedata.current_line().width = typeset.x;

		for (size_t i = 0; i < pagedata.lines.size(); ++i)
		{
			const PageBuildData::Line line = pagedata.lines[i];
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
		TypesetData typeset = {};
		for (size_t i = 0; i < glyph_groups.size(); ++i)
		{
			GlyphGroup::WriteResult result = glyph_groups[i].write_glyph_section(*this, pagedata, typeset, peek_next(i), get_alignment_cache());
			if (result == GlyphGroup::WriteResult::BREAK)
				break;
		}
	}

	Paragraph::AlignmentCache Paragraph::get_alignment_cache() const
	{
		AlignmentCache alignment_cache{};
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
				const PageBuildData::Line line = pagedata.lines[i];
				if (line.space_width > 0.0f)
					alignment_cache.lines[i].space_width_mult = 1.0f + (page_layout.fitted_size.x - line.width) / line.space_width;
			}
			break;
		case ParagraphFormat::HorizontalAlignment::FULL_JUSTIFY:
			for (size_t i = 0; i < alignment_cache.lines.size(); ++i)
			{
				const PageBuildData::Line line = pagedata.lines[i];
				alignment_cache.lines[i].character_shift = line.characters > 1 ? (page_layout.fitted_size.x - line.width) / (line.characters - 1) : 0.0f;
			}
			break;
		}

		// page offsets
		alignment_cache.pivot_offset = page_layout.fitted_size * (glm::vec2{ 0.0f, 1.0f } - format.pivot);
		alignment_cache.padding_offset = format.padding * glm::vec2{ 1.0f, -1.0f };

		return alignment_cache;
	}

	Paragraph::GlyphGroup::PeekData Paragraph::peek_next(size_t i) const
	{
		if (i + 1 >= glyph_groups.size())
			return {};

		const GlyphGroup& current_glyph = glyph_groups[i];
		const GlyphGroup& next_glyph = glyph_groups[i + 1];

		if (current_glyph.element.font->font_face() == next_glyph.element.font->font_face())
			return next_glyph.peek();
		else
			return {};
	}
}
