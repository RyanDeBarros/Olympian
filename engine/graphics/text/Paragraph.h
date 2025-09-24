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

	struct TextElement
	{
		// TODO v5 add scale as an alternative to setting font size without requiring new rasterization. add y pivot - if text element is smaller than line height, should it be align to top, middle, or bottom.
		FontAtlasRef font;
		utf::String text = "";
		glm::vec4 text_color = glm::vec4(1.0f);
		float adj_offset = 0.0f;
	};

	/*
	* A Paragraph represents a collection of glyphs that use the same transform and layout, under a common font.
	*/
	class Paragraph
	{
		struct PageData
		{
			float width = 0.0f, height = 0.0f;
			float content_width = 0.0f, content_height = 0.0f;

			struct Line
			{
				float width = 0.0f;
				float height = 0.0f;
				float spaces = 0.0f;
				float final_advance = 0.0f;

				void fit_height(float h)
				{
					height = glm::max(height, h);
				}
			};
			std::vector<Line> lines;

			float blank_lines = 0.0f;
		};

		struct TypesetData
		{
			float x = 0.0f, y = 0.0f;
			size_t line = 0;
		};

		mutable Sprite bkg;
		ParagraphFormat format;

		struct GlyphGroup
		{
			TextElement element;
			mutable std::vector<TextGlyph> glyphs;

			GlyphGroup(TextElement&& element);

			void set_batch(SpriteBatch* batch);

			void draw() const;

			utf::Codepoint first_codepoint() const;
			void build_page_section(const Paragraph& paragraph, PageData& pagedata, TypesetData& typeset, utf::Codepoint next_first_codepoint) const;
			void write_glyph_section(const Paragraph& paragraph, const PageData& pagedata, TypesetData& typeset, utf::Codepoint next_first_codepoint) const;

		private:
			void build_space(PageData& pagedata, TypesetData& typeset, utf::Codepoint next_codepoint) const;
			void build_tab(PageData& pagedata, const ParagraphFormat& format, TypesetData& typeset, utf::Codepoint next_codepoint) const;
			bool build_newline(PageData& pagedata, const ParagraphFormat& format, TypesetData& typeset) const;
			void build_glyph(PageData& pagedata, TypesetData& typeset, utf::Codepoint c, float dx) const;

			void write_space(const PageData& pagedata, const ParagraphFormat& format, TypesetData& typeset, utf::Codepoint next_codepoint) const;
			void write_tab(const PageData& pagedata, const ParagraphFormat& format, TypesetData& typeset, utf::Codepoint next_codepoint) const;
			bool write_newline(const PageData& pagedata, const ParagraphFormat& format, TypesetData& typeset) const;
			void write_glyph(const Paragraph& paragraph, const PageData& pagedata, TypesetData& typeset, utf::Codepoint c, float dx) const;

			float line_height(const ParagraphFormat& format) const;
			
			float space_width_mult(const PageData& pagedata, const ParagraphFormat& format, const TypesetData& typeset) const;
			float linebreak_mult(const PageData& pagedata, const ParagraphFormat& format) const;
			
			float space_width(utf::Codepoint next_codepoint) const;
			float tab_width(const ParagraphFormat& format, utf::Codepoint next_codepoint) const;
			float advance_width(const FontGlyph& font_glyph, utf::Codepoint codepoint, utf::Codepoint next_codepoint) const;

			TextGlyph create_glyph(const Paragraph& paragraph) const;
			void set_glyph_attributes() const;
		};
		std::vector<GlyphGroup> glyph_groups;

		mutable size_t glyphs_drawn = 0;
		// TODO v5 perhaps keep dirty flag per glyph group to individually update text colors, etc.
		mutable bool dirty_layout = true;
		void flag_dirty() { dirty_layout = true; }

		mutable glm::vec2 page_size = {};

	public:
		bool draw_bkg = false;
		// TODO v5 use exposure, since transformer is now mutable (since glyphs need to attach dynamically in draw()).
		mutable Transformer2D transformer;

		Paragraph(std::vector<TextElement>&& elements, const ParagraphFormat& format = {});
		Paragraph(SpriteBatch* batch, std::vector<TextElement>&& elements, const ParagraphFormat& format = {});

	private:
		void init(std::vector<TextElement>&& elements);

	public:
		SpriteBatch* get_batch() const { return bkg.get_batch(); }
		void set_batch(SpriteBatch* batch);

		const ParagraphFormat& get_format() const;
		ParagraphFormat& set_format();
		glm::vec4 get_bkg_color() const;
		void set_bkg_color(glm::vec4 color);

		const TextElement& get_element(size_t i = 0) const;
		TextElement& set_element(size_t i = 0);
		size_t get_element_count() const;
		void add_element(TextElement&& element);
		void insert_element(size_t i, TextElement&& element);
		void erase_element(size_t i);

		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { return transformer.set_local(); }

		float width() const { return page_size.x; }
		float height() const { return page_size.y; }
		glm::vec2 size() const { return page_size; }

		void draw() const;

	private:
		void build_layout() const;
		PageData build_page() const;
		void write_glyphs(const PageData& pagedata) const;
		utf::Codepoint next_first_codepoint(size_t i) const;
	};

	typedef SmartReference<Paragraph> ParagraphRef;
}
