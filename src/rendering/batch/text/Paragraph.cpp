#include "Paragraph.h"

namespace oly
{
	namespace rendering
	{
		Paragraph::Paragraph(TextBatch& text_batch, const FontAtlasRes& font, utf::String&& text)
			: text_batch(&text_batch), font(font)
		{
			if (!text.empty())
				set_text(std::move(text));
		}
		
		void Paragraph::set_text(utf::String&& text)
		{
			this->text = std::move(text);
			build_layout();
		}

		void Paragraph::set_text(const utf::String& text)
		{
			this->text = text;
			build_layout();
		}

		void Paragraph::set_font(const FontAtlasRes& font)
		{
			this->font = font;
			auto iter = text.begin();
			size_t i = 0;
			while (iter)
			{
				utf::Codepoint codepoint = iter.advance();
				GlyphText& glyph = renderables[i++].glyph;
				if (font->cache(codepoint))
					glyph.set_texture(font->get_glyph(codepoint).texture);
				else
					glyph.set_texture(nullptr);
			}
		}

		TextBatch::Foreground Paragraph::get_foreground_color(size_t pos) const
		{
			return renderables[pos].glyph.get_foreground();
		}

		void Paragraph::set_foreground_color(size_t pos, const TextBatch::Foreground& color)
		{
			renderables[pos].glyph.set_foreground(color);
		}

		TextBatch::Background Paragraph::get_background_color(size_t pos) const
		{
			return renderables[pos].glyph.get_background();
		}

		void Paragraph::set_background_color(size_t pos, const TextBatch::Background& color)
		{
			renderables[pos].glyph.set_background(color);
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
			TypesetData typeset{};
			size_t i = 0;
			auto iter = text.begin();
			while (iter)
			{
				utf::Codepoint codepoint = iter.advance();
				if (i >= renderables.size())
					create_glyph(); // TODO batch create glyphs instead of one at a time?
				Renderable& renderable = renderables[i++];

				if (codepoint == ' ')
				{
					// TODO
				}
				else if (codepoint == '\t')
				{
					// TODO
				}
				else if (utf::is_rn(codepoint, iter ? iter.codepoint() : utf::Codepoint(0)))
				{
					// TODO
					++iter;
				}
				else if (utf::is_n_or_r(codepoint))
				{
					// TODO
				}
				else if (font->cache(codepoint))
				{
					// TODO
					const FontGlyph& font_glyph = font->get_glyph(codepoint);
					renderable.visible = true;
					write_glyph(renderable.glyph, font_glyph, { typeset.x, typeset.y });
					typeset.x += (int)roundf(font_glyph.advance_width * font->get_scale());
				}
				else
					renderable.visible = false;
			}
			text_length = text.size();
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
			glyph.set_vertex_positions({ 0, (float)font_glyph.width, (float)-font_glyph.height, 0 });
			glyph.set_tex_coords(font->uvs(font_glyph));
			glyph.set_local().position = { par_pos.x, par_pos.y };
		}
	}
}
