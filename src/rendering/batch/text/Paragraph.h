#pragma once

#include "Text.h"
#include "Font.h"

namespace oly
{
	namespace rendering
	{
		/*
		* A Paragraph represents a collection of glyphs that use the same transform and layout, under a common font.
		*/
		class Paragraph
		{
			struct Renderable
			{
				GlyphText glyph;
				bool visible;
			};
			std::vector<Renderable> renderables;
			utf::String text;
			mutable size_t text_length = 0;
			TextBatch* text_batch;
			FontAtlasRes font;

		public:
			Transformer2D transformer;

			Paragraph(TextBatch& text_batch, const FontAtlasRes& font, utf::String&& text = "");

			const utf::String& get_text() const { return text; }
			void set_text(utf::String&& text);
			void set_text(const utf::String& text);
			const FontAtlasRes& get_font() const { return font; }
			void set_font(const FontAtlasRes& font);
			const Transform2D& get_local() const { return transformer.get_local(); }
			Transform2D& set_local() { return transformer.set_local(); }

			TextBatch::Foreground get_foreground_color(size_t pos) const;
			void set_foreground_color(size_t pos, const TextBatch::Foreground& color);
			TextBatch::Background get_background_color(size_t pos) const;
			void set_background_color(size_t pos, const TextBatch::Background& color);
			TextBatch::Modulation get_modulation(size_t pos) const;
			void set_modulation(size_t pos, const TextBatch::Modulation& modulation);
			bool is_visible(size_t pos) const;
			void set_visible(size_t pos, bool visible);

			void draw() const;

		private:
			void build_layout();

			void create_glyph();
			void write_glyph(GlyphText& glyph, const FontGlyph& font_glyph, glm::vec2 par_pos) const;

			struct TypesetData
			{
				int x = 0, y = 0;
			};
		};
	}
}
