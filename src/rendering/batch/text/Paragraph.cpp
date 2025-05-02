#include "Paragraph.h"

#include "util/Errors.h"

namespace oly
{
	namespace rendering
	{
		Paragraph::Paragraph(TextBatch& text_batch, const FontAtlasRes& font, const ParagraphFormat format, utf::String&& text)
			: text_batch(&text_batch), format(format), font(font)
		{
			if (!text.empty())
				set_text(std::move(text));
		}

		TextBatch::TextColor Paragraph::get_text_color(size_t pos) const
		{
			return glyphs[pos].get_text_color();
		}

		void Paragraph::set_text_color(size_t pos, const TextBatch::TextColor& color)
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
			for (size_t i = 0; i < glyphs_drawn; ++i)
				if (visible[i])
					glyphs[i].draw();
		}

		void Paragraph::build_layout()
		{
			build_page();
			write_glyphs();
		}

		void Paragraph::build_page()
		{
			pagedata = {};
			pagedata.lines.push_back({});
			typeset = {};
			auto iter = text.begin();
			while (iter)
			{
				utf::Codepoint codepoint = iter.advance();
				if (codepoint == ' ')
				{
					float dx = font->get_space_width();
					typeset.x += dx;
					pagedata.lines.back().width += dx;
					pagedata.lines.back().spaces += dx;
					pagedata.lines.back().final_advance = dx;
				}
				else if (codepoint == '\t')
				{
					float dx = font->get_space_width() * format.tab_spaces;
					typeset.x += dx;
					pagedata.lines.back().width += dx;
					pagedata.lines.back().spaces += dx;
					pagedata.lines.back().final_advance = dx;
				}
				else if (utf::is_n_or_r(codepoint))
				{
					if (iter && utf::is_rn(codepoint, iter.codepoint()))
						++iter;
					float dy;
					if (typeset.x == 0.0f)
						dy = line_height() * format.linebreak_spacing;
					else
						dy = line_height();
					if (format.max_height > 0.0f && typeset.y - dy < -format.max_height)
						break;
					++typeset.line;
					typeset.y -= dy;
					if (typeset.x == 0.0f)
						pagedata.blank_lines += line_height();
					typeset.x = 0.0f;
					pagedata.lines.push_back({});
				}
				else if (font->cache(codepoint))
				{
					const FontGlyph& font_glyph = font->get_glyph(codepoint);
					float dx = font_glyph.advance_width * font->get_scale();
					if (format.text_wrap > 0.0f && typeset.x + dx > format.text_wrap)
					{
						float dy;
						if (typeset.x == 0.0f)
							dy = line_height() * format.linebreak_spacing;
						else
							dy = line_height();
						if (format.max_height > 0.0f && typeset.y - dy < -format.max_height)
							break;
						++typeset.line;
						typeset.y -= dy;
						if (typeset.x == 0.0f)
							pagedata.blank_lines += line_height();
						typeset.x = 0.0f;
						pagedata.lines.push_back({});
					}
					typeset.x += dx;
					pagedata.lines.back().width += dx;
					pagedata.lines.back().final_advance = dx;
				}
				pagedata.width = std::max(pagedata.width, typeset.x);
			}
			pagedata.height = content_height();

			if (pagedata.width < format.min_size.x)
				pagedata.width = format.min_size.x;
			if (pagedata.height < format.min_size.y)
				pagedata.height = format.min_size.y;
		}

		void Paragraph::write_glyphs()
		{
			glyphs_drawn = 0;
			typeset = {};
			typeset.x = 0.0f;
			auto iter = text.begin();
			while (iter)
			{
				utf::Codepoint codepoint = iter.advance();

				if (codepoint == ' ')
					typeset.x += font->get_space_width() * space_width_mult();
				else if (codepoint == '\t')
					typeset.x += font->get_space_width() * space_width_mult() * format.tab_spaces;
				else if (utf::is_n_or_r(codepoint))
				{
					if (iter)
					{
						utf::Codepoint next_char = iter.codepoint();
						if (utf::is_rn(codepoint, next_char))
							++iter;
					}
					float dy;
					if (typeset.x == 0.0f)
						dy = line_height() * linebreak_mult();
					else
						dy = line_height();
					if (format.max_height > 0.0f && typeset.y - dy < -format.max_height)
						break;
					++typeset.line;
					typeset.y -= dy;
					typeset.x = 0.0f;
				}
				else if (font->cache(codepoint))
				{
					const FontGlyph& font_glyph = font->get_glyph(codepoint);
					float dx = font_glyph.advance_width * font->get_scale();
					if (format.text_wrap > 0.0f && typeset.x + dx > format.text_wrap)
					{
						float dy;
						if (typeset.x == 0.0f)
							dy = line_height() * linebreak_mult();
						else
							dy = line_height();
						if (format.max_height > 0.0f && typeset.y - dy < -format.max_height)
							break;
						++typeset.line;
						typeset.y -= dy;
						typeset.x = 0.0f;
					}
					write_glyph(font_glyph);
					typeset.x += dx;
				}
			}
		}

		void Paragraph::create_glyph()
		{
			TextGlyph glyph(*text_batch);
			glyph.transformer.attach_parent(&transformer);
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
			return (pagedata.lines.size() - 1) * line_height() + font->line_height();
		}

		float Paragraph::linebreak_mult() const
		{
			if (format.vertical_alignment == ParagraphFormat::VerticalAlignment::JUSTIFY)
			{
				if (pagedata.blank_lines > 0)
					return 1.0f + (pagedata.height - content_height()) / pagedata.blank_lines;
			}
			return format.linebreak_spacing;
		}
	}
}
