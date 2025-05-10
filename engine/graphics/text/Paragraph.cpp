#include "Paragraph.h"

#include "core/base/Errors.h"
#include "graphics/resources/Textures.h"

namespace oly::rendering
{
	Paragraph::Paragraph(TextBatch& text_batch, const FontAtlasRes& font, const ParagraphFormat& format, utf::String&& text)
		: text_batch(&text_batch), format(format), font(font), bkg(text_batch)
	{
		bkg.transformer.attach_parent(&transformer);
		bkg.transformer.modifier = std::make_unique<oly::PivotTransformModifier2D>();
		bkg.set_texture(graphics::textures::white1x1_1);

		set_bkg_color({ { 0.0f, 0.0f, 0.0f, 1.0f } });
		if (!text.empty())
			set_text(std::move(text));
	}

	void Paragraph::recolor_text_with_default()
	{
		for (auto& glyph : glyphs)
			glyph.set_text_color(default_text_color);
	}

	TextBatch::TextColor Paragraph::get_glyph_color(size_t pos) const
	{
		return glyphs[pos].get_text_color();
	}

	void Paragraph::set_glyph_color(size_t pos, const TextBatch::TextColor& color)
	{
		glyphs[pos].set_text_color(color);
	}

	TextBatch::Modulation Paragraph::get_modulation(size_t pos) const
	{
		return glyphs[pos].get_modulation();
	}

	void Paragraph::set_modulation(size_t pos, const TextBatch::Modulation& modulation)
	{
		glyphs[pos].set_modulation(modulation);
	}

	TextBatch::TextColor Paragraph::get_bkg_color() const
	{
		return bkg.get_text_color();
	}

	void Paragraph::set_bkg_color(const TextBatch::TextColor& color)
	{
		bkg.set_text_color(color);
	}

	TextBatch::Modulation Paragraph::get_bkg_modulation() const
	{
		return bkg.get_modulation();
	}

	void Paragraph::set_bkg_modulation(const TextBatch::Modulation& modulation)
	{
		bkg.set_modulation(modulation);
	}

	bool Paragraph::is_visible(size_t pos) const
	{
		return visible[pos];
	}

	void Paragraph::set_visible(size_t pos, bool visible)
	{
		this->visible[pos] = visible;
	}

	void Paragraph::draw() const
	{
		if (draw_bkg)
			bkg.draw();
		for (size_t i = 0; i < glyphs_drawn; ++i)
			if (visible[i])
				glyphs[i].draw();
	}

	void Paragraph::build_layout()
	{
		build_page();
		write_glyphs();
		auto& bkg_modifier = bkg.transformer.get_modifier<oly::PivotTransformModifier2D>();
		bkg_modifier.size = size() + 2.0f * format.padding;
		bkg_modifier.pivot = format.pivot;
		math::Rect2D vps{};
		vps.x1 = -0.5f * width() - format.padding.x;
		vps.x2 = -vps.x1;
		vps.y1 = -0.5f * height() - format.padding.y;
		vps.y2 = -vps.y1;
		bkg.set_vertex_positions(vps);
	}

	void Paragraph::build_page()
	{
		pagedata = {};
		pagedata.lines.push_back({});
		typeset_text(&Paragraph::build_space, &Paragraph::build_tab, &Paragraph::build_newline, &Paragraph::build_glyph);
		pagedata.height = content_height();
		if (pagedata.width < format.min_size.x)
			pagedata.width = format.min_size.x;
		if (pagedata.height < format.min_size.y)
			pagedata.height = format.min_size.y;
	}

	void Paragraph::write_glyphs()
	{
		glyphs_drawn = 0;
		typeset_text(&Paragraph::write_space, &Paragraph::write_tab, &Paragraph::write_newline, &Paragraph::write_glyph);
	}

	void Paragraph::typeset_text(void(Paragraph::* space)(), void(Paragraph::* tab)(), bool(Paragraph::* newline)(), void(Paragraph::* glyph)(utf::Codepoint, float dx))
	{
		typeset = {};
		auto iter = text.begin();
		while (iter)
		{
			utf::Codepoint codepoint = iter.advance();
			if (codepoint == ' ')
				(this->*space)();
			else if (codepoint == '\t')
				(this->*tab)();
			else if (utf::is_n_or_r(codepoint))
			{
				if (iter && utf::is_rn(codepoint, iter.codepoint()))
					++iter;
				if (!(this->*newline)())
					break;
			}
			else if (font->cache(codepoint))
			{
				const FontGlyph& font_glyph = font->get_glyph(codepoint);
				float dx = advance_width(font_glyph, codepoint);
				if (format.text_wrap > 0.0f && typeset.x + dx > format.text_wrap)
				{
					if ((this->*newline)())
						break;
				}
				(this->*glyph)(codepoint, dx);
			}
		}
	}

	void Paragraph::build_space()
	{
		float dx = space_width();
		typeset.x += dx;
		pagedata.lines.back().width += dx;
		pagedata.lines.back().spaces += dx;
		pagedata.lines.back().final_advance = dx;
		pagedata.width = std::max(pagedata.width, typeset.x);
	}

	void Paragraph::build_tab()
	{
		float dx = tab_width();
		typeset.x += dx;
		pagedata.lines.back().width += dx;
		pagedata.lines.back().spaces += dx;
		pagedata.lines.back().final_advance = dx;
		pagedata.width = std::max(pagedata.width, typeset.x);
	}

	bool Paragraph::build_newline()
	{
		float dy;
		if (typeset.x == 0.0f)
			dy = line_height() * format.linebreak_spacing;
		else
			dy = line_height();
		if (format.max_height > 0.0f && typeset.y - dy < -format.max_height)
			return false;
		++typeset.line;
		typeset.y -= dy;
		if (typeset.x == 0.0f)
			pagedata.blank_lines += line_height();
		typeset.x = 0.0f;
		typeset.prev_codepoint = utf::Codepoint(0);
		pagedata.lines.push_back({});
		return true;
	}

	void Paragraph::build_glyph(utf::Codepoint c, float dx)
	{
		typeset.x += dx;
		pagedata.lines.back().width += dx;
		pagedata.lines.back().final_advance = dx;
		pagedata.width = std::max(pagedata.width, typeset.x);
	}

	void Paragraph::write_space()
	{
		typeset.x += space_width() * space_width_mult();
	}

	void Paragraph::write_tab()
	{
		typeset.x += tab_width() * space_width_mult();
	}

	bool Paragraph::write_newline()
	{
		float dy;
		if (typeset.x == 0.0f)
			dy = line_height() * linebreak_mult();
		else
			dy = line_height();
		if (format.max_height > 0.0f && typeset.y - dy < -format.max_height)
			return false;
		++typeset.line;
		typeset.y -= dy;
		typeset.x = 0.0f;
		typeset.prev_codepoint = utf::Codepoint(0);
		return true;
	}

	void Paragraph::write_glyph(utf::Codepoint c, float dx)
	{
		write_glyph(font->get_glyph(c));
		typeset.x += dx;
	}

	void Paragraph::create_glyph()
	{
		TextGlyph glyph(*text_batch);
		glyph.transformer.attach_parent(&transformer);
		glyph.set_text_color(default_text_color);
		glyphs.emplace_back(std::move(glyph));
		visible.push_back(true);
	}

	void Paragraph::write_glyph(const FontGlyph& font_glyph)
	{
		if (glyphs_drawn >= glyphs.size())
			create_glyph();
		TextGlyph& glyph = glyphs[glyphs_drawn];

		glyph.set_texture(font_glyph.texture);
		math::Rect2D vps{};
		vps.x1 = (float) font_glyph.box.x1;
		vps.x2 = (float) font_glyph.box.x2;
		vps.y1 = (float)-font_glyph.box.y2;
		vps.y2 = (float)-font_glyph.box.y1;
		glyph.set_vertex_positions(vps);
		glyph.set_tex_coords(font->uvs(font_glyph));

		float line_start_x = 0.0f;
		if (format.horizontal_alignment == ParagraphFormat::HorizontalAlignment::RIGHT)
			line_start_x = pagedata.width - pagedata.lines[typeset.line].width;
		else if (format.horizontal_alignment == ParagraphFormat::HorizontalAlignment::CENTER)
			line_start_x = 0.5f * (pagedata.width - pagedata.lines[typeset.line].width);

		float line_start_y = 0.0f;
		if (format.vertical_alignment == ParagraphFormat::VerticalAlignment::BOTTOM)
			line_start_y = pagedata.height - content_height();
		else if (format.vertical_alignment == ParagraphFormat::VerticalAlignment::MIDDLE)
			line_start_y = 0.5f * (pagedata.height - content_height());

		float typeset_mult_x = 1.0f;
		if (format.horizontal_alignment == ParagraphFormat::HorizontalAlignment::FULL_JUSTIFY)
		{
			PageData::Line ln = pagedata.lines[typeset.line];
			if (ln.width - ln.final_advance > 0.0f)
				typeset_mult_x = (pagedata.width - ln.final_advance) / (ln.width - ln.final_advance);
		}
			
		float typeset_mult_y = 1.0f;
		if (format.vertical_alignment == ParagraphFormat::VerticalAlignment::FULL_JUSTIFY)
		{
			if (content_height() - font->line_height() > 0.0f)
				typeset_mult_y = (pagedata.height - font->line_height()) / (content_height() - font->line_height());
		}

		glyph.set_local().position.x = line_start_x + typeset.x * typeset_mult_x - format.pivot.x * pagedata.width + format.padding.x;
		glyph.set_local().position.y = -line_start_y + typeset.y * typeset_mult_y - font->get_ascent() + (1.0f - format.pivot.y) * pagedata.height - format.padding.y;
		++glyphs_drawn;
	}

	float Paragraph::line_height() const
	{
		return font->line_height() * format.line_spacing;
	}

	float Paragraph::space_width_mult() const
	{
		if (format.horizontal_alignment == ParagraphFormat::HorizontalAlignment::JUSTIFY)
		{
			PageData::Line ln = pagedata.lines[typeset.line];
			if (ln.spaces > 0.0f)
				return 1.0f + (pagedata.width - ln.width) / ln.spaces;
		}
		return 1.0f;
	}

	float Paragraph::content_height() const
	{
		return (pagedata.lines.size() - 1) * line_height() + pagedata.blank_lines * (format.linebreak_spacing - 1.0f) + font->line_height();
	}

	float Paragraph::linebreak_mult() const
	{
		if (format.vertical_alignment == ParagraphFormat::VerticalAlignment::JUSTIFY)
		{
			if (pagedata.blank_lines > 0)
				return 1.0f + (pagedata.height - content_height() + line_height()) / pagedata.blank_lines;
		}
		return format.linebreak_spacing;
	}

	float Paragraph::space_width()
	{
		float adv = font->get_space_width();
		if (typeset.prev_codepoint)
			adv += font->kerning_of(typeset.prev_codepoint, utf::Codepoint(' '));
		typeset.prev_codepoint = utf::Codepoint(' ');
		return adv;
	}

	float Paragraph::tab_width()
	{
		float first = space_width();
		float rest = space_width() * (format.tab_spaces - 1.0f);
		return first + rest;
	}
		
	float Paragraph::advance_width(const FontGlyph& font_glyph, utf::Codepoint codepoint)
	{
		float adv = font_glyph.advance_width * font->get_scale();
		if (typeset.prev_codepoint)
			adv += font->kerning_of(typeset.prev_codepoint, codepoint, font->get_glyph_index(typeset.prev_codepoint), font_glyph.index);
		typeset.prev_codepoint = codepoint;
		return adv;
	}
}
