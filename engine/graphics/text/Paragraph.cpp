#include "Paragraph.h"

#include "core/base/Errors.h"
#include "core/context/rendering/Fonts.h"
#include "core/context/rendering/Sprites.h"
#include "core/context/rendering/Textures.h"
#include "graphics/resources/Textures.h"

namespace oly::rendering
{
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
	
	utf::Codepoint Paragraph::GlyphGroup::first_codepoint() const
	{
		auto iter = element.text.begin();
		return iter ? iter.codepoint() : utf::Codepoint(0);
	}

	// TODO v5 extract common process in build_page_section/write_glyph_section

	void Paragraph::GlyphGroup::build_page_section(const Paragraph& paragraph, PageData& pagedata, TypesetData& typeset, utf::Codepoint next_first_codepoint) const
	{
		auto iter = element.text.begin();
		if (iter)
			pagedata.lines.back().fit_height(element.font->line_height());

		if (element.adj_offset > 0.0f && typeset.x > 0.0f)
		{
			if (paragraph.format.text_wrap > 0.0f && typeset.x + element.adj_offset > paragraph.format.text_wrap)
			{
				if (!build_newline(pagedata, paragraph.format, typeset))
					return;
			}
			else
				typeset.x += element.adj_offset;
		}

		while (iter)
		{
			utf::Codepoint codepoint = iter.advance();
			utf::Codepoint next_codepoint = iter ? iter.codepoint() : next_first_codepoint;

			if (codepoint == ' ')
				build_space(pagedata, typeset, next_codepoint);
			else if (codepoint == '\t')
				build_tab(pagedata, paragraph.format, typeset, next_codepoint);
			else if (utf::is_n_or_r(codepoint))
			{
				if (iter && utf::is_rn(codepoint, next_codepoint))
					++iter;
				if (build_newline(pagedata, paragraph.format, typeset))
				{
					if (next_codepoint)
						pagedata.lines.back().fit_height(element.font->line_height());
				}
				else
					break;
			}
			else if (element.font->cache(codepoint))
			{
				const FontGlyph& font_glyph = element.font->get_glyph(codepoint);
				float dx = advance_width(font_glyph, codepoint, next_codepoint);
				if (paragraph.format.text_wrap > 0.0f && typeset.x + dx > paragraph.format.text_wrap)
				{
					if (!build_newline(pagedata, paragraph.format, typeset))
						break;
				}
				build_glyph(pagedata, typeset, codepoint, dx);
			}
		}
	}

	void Paragraph::GlyphGroup::write_glyph_section(const Paragraph& paragraph, const PageData& pagedata, TypesetData& typeset, utf::Codepoint next_first_codepoint) const
	{
		glyphs.clear();

		if (element.adj_offset > 0.0f && typeset.x > 0.0f)
		{
			if (paragraph.format.text_wrap > 0.0f && typeset.x + element.adj_offset > paragraph.format.text_wrap)
			{
				if (!write_newline(pagedata, paragraph.format, typeset))
					return;
			}
			else
				typeset.x += element.adj_offset;
		}

		auto iter = element.text.begin();
		while (iter)
		{
			utf::Codepoint codepoint = iter.advance();
			utf::Codepoint next_codepoint = iter ? iter.codepoint() : next_first_codepoint;

			if (codepoint == ' ')
				write_space(pagedata, paragraph.format, typeset, next_codepoint);
			else if (codepoint == '\t')
				write_tab(pagedata, paragraph.format, typeset, next_codepoint);
			else if (utf::is_n_or_r(codepoint))
			{
				if (iter && utf::is_rn(codepoint, next_codepoint))
					++iter;
				if (!write_newline(pagedata, paragraph.format, typeset))
					break;
			}
			else if (element.font->cache(codepoint))
			{
				const FontGlyph& font_glyph = element.font->get_glyph(codepoint);
				float dx = advance_width(font_glyph, codepoint, next_codepoint);
				if (paragraph.format.text_wrap > 0.0f && typeset.x + dx > paragraph.format.text_wrap)
				{
					if (!write_newline(pagedata, paragraph.format, typeset))
						break;
				}
				write_glyph(paragraph, pagedata, typeset, codepoint, dx);
			}
		}

		set_glyph_attributes(); // TODO v5 if changing text color, no need to re-build/write entire section - just call set_glyph_attributes. use separate dirty flag
	}

	void Paragraph::GlyphGroup::build_space(PageData& pagedata, TypesetData& typeset, utf::Codepoint next_codepoint) const
	{
		float dx = space_width(next_codepoint);
		typeset.x += dx;
		pagedata.lines.back().width += dx;
		pagedata.lines.back().spaces += dx;
		pagedata.lines.back().final_advance = dx;
		pagedata.width = std::max(pagedata.width, typeset.x);
	}

	void Paragraph::GlyphGroup::build_tab(PageData& pagedata, const ParagraphFormat& format, TypesetData& typeset, utf::Codepoint next_codepoint) const
	{
		float dx = tab_width(format, next_codepoint);
		typeset.x += dx;
		pagedata.lines.back().width += dx;
		pagedata.lines.back().spaces += dx;
		pagedata.lines.back().final_advance = dx;
		pagedata.width = std::max(pagedata.width, typeset.x);
	}

	bool Paragraph::GlyphGroup::build_newline(PageData& pagedata, const ParagraphFormat& format, TypesetData& typeset) const
	{
		float dy;
		if (typeset.x == 0.0f)
			dy = line_height(format) * format.linebreak_spacing;
		else
			dy = line_height(format);
		if (format.max_height > 0.0f && typeset.y - dy < -format.max_height)
			return false;

		++typeset.line;
		typeset.y -= dy;
		if (typeset.x == 0.0f)
		{
			pagedata.height += line_height(format) * (format.linebreak_spacing - 1.0f);
			pagedata.blank_lines += line_height(format);
		}
		typeset.x = 0.0f;

		pagedata.lines.back().fit_height(dy);
		pagedata.lines.push_back({});
		return true;
	}

	void Paragraph::GlyphGroup::build_glyph(PageData& pagedata, TypesetData& typeset, utf::Codepoint c, float dx) const
	{
		typeset.x += dx;
		pagedata.lines.back().width += dx;
		pagedata.lines.back().final_advance = dx;
		pagedata.width = std::max(pagedata.width, typeset.x);
	}

	void Paragraph::GlyphGroup::write_space(const PageData& pagedata, const ParagraphFormat& format, TypesetData& typeset, utf::Codepoint next_codepoint) const
	{
		typeset.x += space_width(next_codepoint) * space_width_mult(pagedata, format, typeset);
	}

	void Paragraph::GlyphGroup::write_tab(const PageData& pagedata, const ParagraphFormat& format, TypesetData& typeset, utf::Codepoint next_codepoint) const
	{
		typeset.x += tab_width(format, next_codepoint) * space_width_mult(pagedata, format, typeset);
	}

	bool Paragraph::GlyphGroup::write_newline(const PageData& pagedata, const ParagraphFormat& format, TypesetData& typeset) const
	{
		float dy;
		if (typeset.x == 0.0f)
			dy = line_height(format) * linebreak_mult(pagedata, format);
		else
			dy = line_height(format);
		if (format.max_height > 0.0f && typeset.y - dy < -format.max_height)
			return false;

		++typeset.line;
		typeset.y -= dy;
		typeset.x = 0.0f;
		return true;
	}

	void Paragraph::GlyphGroup::write_glyph(const Paragraph& paragraph, const PageData& pagedata, TypesetData& typeset, utf::Codepoint c, float dx) const
	{
		float line_start_x = 0.0f;
		if (paragraph.format.horizontal_alignment == ParagraphFormat::HorizontalAlignment::RIGHT)
			line_start_x = pagedata.width - pagedata.lines[typeset.line].width;
		else if (paragraph.format.horizontal_alignment == ParagraphFormat::HorizontalAlignment::CENTER)
			line_start_x = 0.5f * (pagedata.width - pagedata.lines[typeset.line].width);

		float line_start_y = 0.0f;
		if (paragraph.format.vertical_alignment == ParagraphFormat::VerticalAlignment::BOTTOM)
			line_start_y = pagedata.height - pagedata.content_height;
		else if (paragraph.format.vertical_alignment == ParagraphFormat::VerticalAlignment::MIDDLE)
			line_start_y = 0.5f * (pagedata.height - pagedata.content_height);

		float typeset_mult_x = 1.0f;
		if (paragraph.format.horizontal_alignment == ParagraphFormat::HorizontalAlignment::FULL_JUSTIFY)
		{
			PageData::Line ln = pagedata.lines[typeset.line];
			if (ln.width - ln.final_advance > 0.0f)
				typeset_mult_x = (pagedata.width - ln.final_advance) / (ln.width - ln.final_advance);
		}

		float typeset_mult_y = 1.0f;
		if (paragraph.format.vertical_alignment == ParagraphFormat::VerticalAlignment::FULL_JUSTIFY)
		{
			if (pagedata.content_height - element.font->line_height() > 0.0f)
				typeset_mult_y = (pagedata.height - element.font->line_height()) / (pagedata.content_height - element.font->line_height());
		}

		glm::vec2 glyph_pos = {
			line_start_x + typeset.x * typeset_mult_x - paragraph.format.pivot.x * pagedata.width + paragraph.format.padding.x,
			-line_start_y + typeset.y * typeset_mult_y - element.font->get_ascent() + (1.0f - paragraph.format.pivot.y) * pagedata.height - paragraph.format.padding.y
		};

		TextGlyph glyph = create_glyph(paragraph);
		glyph.set_glyph(*element.font, element.font->get_glyph(c), glyph_pos);
		glyphs.emplace_back(std::move(glyph));

		typeset.x += dx;
	}

	float Paragraph::GlyphGroup::line_height(const ParagraphFormat& format) const
	{
		return element.font->line_height() * format.line_spacing;
	}

	float Paragraph::GlyphGroup::space_width_mult(const PageData& pagedata, const ParagraphFormat& format, const TypesetData& typeset) const
	{
		if (format.horizontal_alignment == ParagraphFormat::HorizontalAlignment::JUSTIFY)
		{
			PageData::Line ln = pagedata.lines[typeset.line];
			if (ln.spaces > 0.0f)
				return 1.0f + (pagedata.width - ln.width) / ln.spaces;
		}
		return 1.0f;
	}

	float Paragraph::GlyphGroup::linebreak_mult(const PageData& pagedata, const ParagraphFormat& format) const
	{
		if (format.vertical_alignment == ParagraphFormat::VerticalAlignment::JUSTIFY)
		{
			if (pagedata.blank_lines > 0)
				return 1.0f + (pagedata.height - pagedata.height + line_height(format)) / pagedata.blank_lines;
		}
		return format.linebreak_spacing;
	}

	float Paragraph::GlyphGroup::space_width(utf::Codepoint next_codepoint) const
	{
		float adv = element.font->get_space_width();
		if (next_codepoint)
			adv += element.font->kerning_of(next_codepoint, utf::Codepoint(' '));
		return adv;
	}

	float Paragraph::GlyphGroup::tab_width(const ParagraphFormat& format, utf::Codepoint next_codepoint) const
	{
		float first = space_width(next_codepoint);
		float rest = space_width(next_codepoint) * (format.tab_spaces - 1.0f);
		return first + rest;
	}

	float Paragraph::GlyphGroup::advance_width(const FontGlyph& font_glyph, utf::Codepoint codepoint, utf::Codepoint next_codepoint) const
	{
		float adv = font_glyph.advance_width * element.font->get_scale();
		if (next_codepoint)
			adv += element.font->kerning_of(codepoint, next_codepoint, font_glyph.index, element.font->get_glyph_index(next_codepoint));
		return adv;
	}

	TextGlyph Paragraph::GlyphGroup::create_glyph(const Paragraph& paragraph) const
	{
		TextGlyph glyph;
		glyph.transformer.attach_parent(&paragraph.transformer);
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

		bkg.transformer.attach_parent(&transformer);
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

	const ParagraphFormat& Paragraph::get_format() const
	{
		return format;
	}

	ParagraphFormat& Paragraph::set_format()
	{
		flag_dirty();
		return format;
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
		flag_dirty();
		return glyph_groups[i].element;
	}

	size_t Paragraph::get_element_count() const
	{
		return glyph_groups.size();
	}
	
	void Paragraph::add_element(TextElement&& element)
	{
		glyph_groups.emplace_back(std::move(element));
		flag_dirty();
	}
	
	void Paragraph::insert_element(size_t i, TextElement&& element)
	{
		glyph_groups.emplace(glyph_groups.begin() + i, std::move(element));
		flag_dirty();
	}
	
	void Paragraph::erase_element(size_t i)
	{
		glyph_groups.erase(glyph_groups.begin() + i);
		flag_dirty();
	}

	void Paragraph::draw() const
	{
		if (dirty_layout)
		{
			build_layout();
			dirty_layout = false;
		}

		if (draw_bkg)
			bkg.draw();
		for (const GlyphGroup& glyph_group : glyph_groups)
			glyph_group.draw();
	}

	void Paragraph::build_layout() const
	{
		write_glyphs(build_page());

		auto& bkg_modifier = bkg.transformer.ref_modifier<PivotTransformModifier2D>();
		bkg_modifier.size = size() + 2.0f * format.padding;
		bkg_modifier.pivot = format.pivot;
		bkg.set_local().scale = bkg_modifier.size;
	}

	Paragraph::PageData Paragraph::build_page() const
	{
		PageData pagedata = {};
		pagedata.lines.push_back({});

		TypesetData typeset = {};
		for (size_t i = 0; i < glyph_groups.size(); ++i)
			glyph_groups[i].build_page_section(*this, pagedata, typeset, next_first_codepoint(i));

		for (const auto& line : pagedata.lines)
			pagedata.height += line.height;

		pagedata.content_width = pagedata.width;
		pagedata.content_height = pagedata.height;

		// TODO v5 remove this check since pagedata.width should never really be greater than min size -> then can remove content_width as well
		if (pagedata.width < format.min_size.x)
			pagedata.width = format.min_size.x;
		if (pagedata.height < format.min_size.y)
			pagedata.height = format.min_size.y;

		page_size = { pagedata.width, pagedata.height };

		return pagedata;
	}

	void Paragraph::write_glyphs(const PageData& pagedata) const
	{
		glyphs_drawn = 0;
		TypesetData typeset = {};
		for (size_t i = 0; i < glyph_groups.size(); ++i)
			glyph_groups[i].write_glyph_section(*this, pagedata, typeset, next_first_codepoint(i));
	}

	utf::Codepoint Paragraph::next_first_codepoint(size_t i) const
	{
		if (i + 1 >= glyph_groups.size())
			return utf::Codepoint(0);

		const GlyphGroup& current_glyph = glyph_groups[i];
		const GlyphGroup& next_glyph = glyph_groups[i + 1];

		if (current_glyph.element.font->font_face() == next_glyph.element.font->font_face())
			return next_glyph.first_codepoint();
		else
			return utf::Codepoint(0);
	}
}
