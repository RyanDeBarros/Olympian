#pragma once

#include "graphics/text/Text.h"
#include "graphics/text/Font.h"
#include "graphics/sprites/Sprite.h"

namespace oly::rendering
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
		Sprite bkg;
		std::vector<TextGlyph> glyphs;
		std::vector<bool> visible;
		utf::String text;
		FontAtlasRef font;
		ParagraphFormat format;

	public:
		bool draw_bkg = false;

	private:
		size_t glyphs_drawn = 0;

	public:
		glm::vec4 default_text_color = glm::vec4(1.0f);
		Transformer2D transformer;

		Paragraph(const FontAtlasRef& font, const ParagraphFormat& format = {}, utf::String&& text = "");
		Paragraph(SpriteBatch* batch, const FontAtlasRef& font, const ParagraphFormat& format = {}, utf::String&& text = "");

	private:
		void init(utf::String&& text);

	public:
		SpriteBatch* get_batch() const { return bkg.get_batch(); }
		void set_batch(SpriteBatch* batch);

		const utf::String& get_text() const { return text; }
		void set_text(utf::String&& text) { this->text = std::move(text); build_layout(); }
		void set_text(const utf::String& text) { this->text = text; build_layout(); }
		const FontAtlasRef& get_font() const { return font; }
		void set_font(const FontAtlasRef& font) { this->font = font; build_layout(); }
		const ParagraphFormat& get_format() const { return format;  }
		void set_format(const ParagraphFormat& format) { this->format = format; build_layout(); }

		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { return transformer.set_local(); }

		void recolor_text_with_default();
		glm::vec4 get_glyph_color(size_t pos) const;
		void set_glyph_color(size_t pos, glm::vec4 color);
		glm::vec4 get_bkg_color() const;
		void set_bkg_color(glm::vec4 color);
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
		void typeset_text(void(Paragraph::* space)(utf::Codepoint next_codepoint), void(Paragraph::* tab)(utf::Codepoint next_codepoint),
			bool(Paragraph::* newline)(), void(Paragraph::* glyph)(utf::Codepoint, float dx));

		void build_space(utf::Codepoint next_codepoint);
		void build_tab(utf::Codepoint next_codepoint);
		bool build_newline();
		void build_glyph(utf::Codepoint c, float dx);

		void write_space(utf::Codepoint next_codepoint);
		void write_tab(utf::Codepoint next_codepoint);
		bool write_newline();
		void write_glyph(utf::Codepoint c, float dx);

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
		float space_width(utf::Codepoint next_codepoint);
		float tab_width(utf::Codepoint next_codepoint);
		float advance_width(const FontGlyph& font_glyph, utf::Codepoint codepoint, utf::Codepoint next_codepoint);
	};

	typedef SmartReference<Paragraph> ParagraphRef;

	// TODO v4 RichText, which uses a vector of strings which can each have their own font, color, etc, but is still formatted as one paragraph. Thus, remove individual glypph colors from regular Paragraph.
}
