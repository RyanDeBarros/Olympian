#pragma once

#include "Text.h"
#include "Font.h"

namespace oly
{
	namespace rendering
	{
		struct ParagraphFormat
		{
			float line_spacing = 1.0f;
			float tab_spaces = 4.0f;
			float linebreak_spacing = 1.0f;
			glm::vec2 pivot = { 0.5f, 0.5f };
			glm::vec2 min_size = {};
			float text_wrap = 0.0f; // a standard glyph is around 42
			float max_height = 0.0f; // a standard line is around 115
			glm::vec2 padding = {};

			enum class HorizontalAlignment
			{
				LEFT,
				CENTER,
				RIGHT,
				JUSTIFY,
				FULL_JUSTIFY
			} horizontal_alignment = HorizontalAlignment::LEFT;

			enum class VerticalAlignment
			{
				TOP,
				MIDDLE,
				BOTTOM,
				JUSTIFY,
				FULL_JUSTIFY
			} vertical_alignment = VerticalAlignment::TOP;
		};

		/*
		* A Paragraph represents a collection of glyphs that use the same transform and layout, under a common font.
		*/
		class Paragraph
		{
			std::vector<TextGlyph> glyphs;
			std::vector<bool> visible;
			utf::String text;
			TextBatch* text_batch;
			FontAtlasRes font;
			ParagraphFormat format;
			size_t glyphs_drawn = 0;

		public:
			Transformer2D transformer;

			Paragraph(TextBatch& text_batch, const FontAtlasRes& font, const ParagraphFormat format = {}, utf::String&& text = "");

			const utf::String& get_text() const { return text; }
			void set_text(utf::String&& text) { this->text = std::move(text); build_layout(); }
			void set_text(const utf::String& text) { this->text = text; build_layout(); }
			const FontAtlasRes& get_font() const { return font; }
			void set_font(const FontAtlasRes& font) { this->font = font; build_layout(); }
			const ParagraphFormat& get_format() const { return format;  }
			void set_format(const ParagraphFormat& format) { this->format = format; build_layout(); }

			const Transform2D& get_local() const { return transformer.get_local(); }
			Transform2D& set_local() { return transformer.set_local(); }

			TextBatch::TextColor get_text_color(size_t pos) const;
			void set_text_color(size_t pos, const TextBatch::TextColor& color);
			TextBatch::Modulation get_modulation(size_t pos) const;
			void set_modulation(size_t pos, const TextBatch::Modulation& modulation);
			bool is_visible(size_t pos) const;
			void set_visible(size_t pos, bool visible);

			float width() const { return pagedata.width; }
			float height() const { return pagedata.height; }
			glm::vec2 size() const { return { pagedata.width, pagedata.height }; }

			void draw() const;

		private:
			void build_layout();
			void build_page();
			void write_glyphs();

			void create_glyph();
			void write_glyph(const FontGlyph& font_glyph);

			float line_height() const;

			struct PageData
			{
				float width = 0.0f, height = 0.0f;

				struct Line
				{
					float width = 0.0f;
					float spaces = 0.0f;
					float final_advance = 0.0f;
				};
				std::vector<Line> lines;

				float blank_lines = 0.0f;
				
			} pagedata = {};
			struct TypesetData
			{
				float x = 0.0f, y = 0.0f;
				size_t line = 0;
			} typeset = {};

			float space_width_mult() const;
			float content_height() const;
			float linebreak_mult() const;
		};
	}
}
