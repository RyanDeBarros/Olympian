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
			return renderables[pos].glyph.get_text_color();
		}

		void Paragraph::set_text_color(size_t pos, const TextBatch::TextColor& color)
		{
			renderables[pos].glyph.set_text_color(color);
		}

		TextBatch::Modulation Paragraph::get_modulation(size_t pos) const
		{
			return renderables[pos].glyph.get_modulation();
		}

		void Paragraph::set_modulation(size_t pos, const TextBatch::Modulation& modulation)
		{
			renderables[pos].glyph.set_modulation(modulation);
		}

		bool Paragraph::is_visible(size_t pos) const
		{
			return renderables[pos].visible;
		}

		void Paragraph::set_visible(size_t pos, bool visible)
		{
			renderables[pos].visible = visible;
		}

		void Paragraph::draw() const
		{
			for (const auto& renderable : renderables)
				if (renderable.visible)
					renderable.glyph.draw();
		}

		void Paragraph::build_layout()
		{
			build_page();
			write_glyphs();
			text_length = text.size();
		}

		void Paragraph::build_page()
		{
			pagedata = {};
			typeset = {};
			auto iter = text.begin();
			while (iter)
			{
				utf::Codepoint codepoint = iter.advance();

				if (codepoint == ' ')
					typeset.x += font->get_space_width();
				else if (codepoint == '\t')
					typeset.x += font->get_space_width() * format.tab_spaces;
				else if (utf::is_n_or_r(codepoint))
				{
					if (iter)
					{
						utf::Codepoint next_char = iter.codepoint();
						if (utf::is_rn(codepoint, next_char))
							++iter;
					}
					newline();
					++pagedata.newlines;
				}
				else if (font->cache(codepoint))
				{
					const FontGlyph& font_glyph = font->get_glyph(codepoint);
					typeset.x += roundi(font_glyph.advance_width * font->get_scale());
				}
				pagedata.width = std::max(pagedata.width, typeset.x);
			}
			pagedata.height = pagedata.newlines * line_height() + font->line_height();
		}

		void Paragraph::write_glyphs()
		{
			typeset = {};
			size_t i = 0;
			auto iter = text.begin();
			while (iter)
			{
				utf::Codepoint codepoint = iter.advance();
				if (i >= renderables.size())
					create_glyph();
				Renderable& renderable = renderables[i++];

				if (codepoint == ' ')
					typeset.x += font->get_space_width();
				else if (codepoint == '\t')
					typeset.x += font->get_space_width() * format.tab_spaces;
				else if (utf::is_n_or_r(codepoint))
				{
					if (iter)
					{
						utf::Codepoint next_char = iter.codepoint();
						if (utf::is_rn(codepoint, next_char))
							++iter;
					}
					newline();
				}
				else if (font->cache(codepoint))
				{
					const FontGlyph& font_glyph = font->get_glyph(codepoint);
					renderable.visible = true;
					write_glyph(renderable.glyph, font_glyph, { typeset.x, typeset.y });
					typeset.x += roundi(font_glyph.advance_width * font->get_scale());
				}
				else
					renderable.visible = false;
			}
		}

		void Paragraph::create_glyph()
		{
			GlyphText glyph(*text_batch);
			glyph.transformer.attach_parent(&transformer);
			renderables.emplace_back(std::move(glyph), false);
		}

		void Paragraph::write_glyph(GlyphText& glyph, const FontGlyph& font_glyph, glm::vec2 par_pos) const
		{
			glyph.set_texture(font_glyph.texture);
			glyph.set_vertex_positions({ (float)font_glyph.box.x1, (float)font_glyph.box.x2, -(float)font_glyph.box.y2, -(float)font_glyph.box.y1});
			glyph.set_tex_coords(font->uvs(font_glyph));
			glyph.set_local().position = { par_pos.x - format.pivot.x * pagedata.width, par_pos.y - font->get_ascent() + (1.0 - format.pivot.y) * pagedata.height};
		}

		float Paragraph::line_height() const
		{
			return font->line_height() * format.line_spacing;
		}

		void Paragraph::newline()
		{
			typeset.x = 0.0f;
			typeset.y -= line_height();
		}
	}
}
