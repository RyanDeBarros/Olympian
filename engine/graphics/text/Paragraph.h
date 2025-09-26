#pragma once

#include "graphics/text/Text.h"
#include "graphics/text/Font.h"
#include "graphics/sprites/Sprite.h"
#include "core/base/TransformerExposure.h"
#include "core/base/Parameters.h"

// TODO v5 RasterTextGlyph and RasterParagraph that does paragraph layout for text that doesn't use font files - pixel art fonts, etc.

namespace oly::rendering
{
	namespace internal
	{
		enum DirtyParagraph
		{
			REBUILD_LAYOUT = 1 << 0,
			HORIZONTAL_ALIGN = 1 << 1,
			VERTICAL_ALIGN = 1 << 2
		};

		inline DirtyParagraph operator~(DirtyParagraph a) { return DirtyParagraph(~(int)a); }

		inline DirtyParagraph operator|(DirtyParagraph a, DirtyParagraph b) { return DirtyParagraph((int)a | (int)b); }
		inline DirtyParagraph& operator|=(DirtyParagraph& a, DirtyParagraph b) { a = a | b; return a; }

		inline DirtyParagraph operator&(DirtyParagraph a, DirtyParagraph b) { return DirtyParagraph((int)a & (int)b); }
		inline DirtyParagraph& operator&=(DirtyParagraph& a, DirtyParagraph b) { a = a & b; return a; }

		enum DirtyGlyphGroup
		{
			RECOLOR = 1 << 0,
			LINE_ALIGNMENT = 1 << 1,
			JITTER_OFFSET = 1 << 2
		};

		inline DirtyGlyphGroup operator~(DirtyGlyphGroup a) { return DirtyGlyphGroup(~(int)a); }

		inline DirtyGlyphGroup operator|(DirtyGlyphGroup a, DirtyGlyphGroup b) { return DirtyGlyphGroup((int)a | (int)b); }
		inline DirtyGlyphGroup& operator|=(DirtyGlyphGroup& a, DirtyGlyphGroup b) { a = a | b; return a; }

		inline DirtyGlyphGroup operator&(DirtyGlyphGroup a, DirtyGlyphGroup b) { return DirtyGlyphGroup((int)a & (int)b); }
		inline DirtyGlyphGroup& operator&=(DirtyGlyphGroup& a, DirtyGlyphGroup b) { a = a & b; return a; }
	}

	class Paragraph;

	struct ParagraphFormat
	{
		float line_spacing = 1.0f;
		float tab_spaces = 4.0f;
		float linebreak_spacing = 1.0f;
		glm::vec2 pivot = { 0.5f, 0.5f };
		glm::vec2 min_size = {};
		float text_wrap = 0.0f;
		float max_height = 0.0f;
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

	struct ParagraphFormatExposure
	{
	private:
		friend class Paragraph;
		Paragraph& paragraph;

		ParagraphFormatExposure(Paragraph& paragraph);

	public:
		void set_line_spacing(float line_spacing);
		void set_tab_spaces(float tab_spaces);
		void set_linebreak_spacing(float linebreak_spacing);
		void set_pivot(glm::vec2 pivot);
		void set_min_size(glm::vec2 min_size);
		void set_text_wrap(float text_wrap);
		void set_max_height(float max_height);
		void set_padding(glm::vec2 padding);
		void set_horizontal_alignment(ParagraphFormat::HorizontalAlignment alignment);
		void set_vertical_alignment(ParagraphFormat::VerticalAlignment alignment);
	};

	struct TextElement
	{
		FontAtlasRef font;
		utf::String text = "";
		glm::vec4 text_color = glm::vec4(1.0f);
		float adj_offset = 0.0f;
		glm::vec2 scale = glm::vec2(1.0f);
		BoundedFloat<0.0f, 1.0f> line_y_pivot = 0.0f;
		glm::vec2 jitter_offset = {};

		float line_height() const { return font->line_height() * scale.y; }
	};

	class TextElementExposure;
	
	namespace internal
	{
		struct PageLayout
		{
			glm::vec2 content_size;
			glm::vec2 fitted_size;
		};

		struct PageBuildData
		{
			struct Line
			{
				float width = 0.0f;
				float max_height = 0.0f;
				float space_width = 0.0f;
				float final_advance_width = 0.0f;
				GLuint characters = 0;

				float spaced_height(const ParagraphFormat& format) const { return max_height * (width == 0.0f ? format.linebreak_spacing : format.line_spacing); }
			};

			std::vector<Line> lines;
			GLuint linebreaks = 0;

			const Line& current_line() const { return lines.back(); }
			Line& current_line() { return lines.back(); }
		};

		struct TypesetData
		{
			float x = 0.0f, y = 0.0f;
			GLuint character = 0;
			size_t line = 0;
		};

		struct AlignmentCache
		{
			struct Line
			{
				float start = 0.0f;
				float character_shift = 0.0f;
				float space_width_mult = 1.0f;
				float height = 0.0f;
			};

			std::vector<Line> lines;

			glm::vec2 pivot_offset = {};
			glm::vec2 padding_offset = {};
			float valign_offset = 0.0f;

			glm::vec2 position(const TypesetData& typeset) const
			{
				return pivot_offset + padding_offset + glm::vec2{ typeset.x, typeset.y } + glm::vec2{
					lines[typeset.line].start + lines[typeset.line].character_shift * typeset.character,
					valign_offset
				};
			}
		};

		class GlyphGroup
		{
			friend class Paragraph;
			friend class TextElementExposure;
			const Paragraph* paragraph = nullptr;
			TextElement element;

			mutable std::vector<TextGlyph> glyphs;

			struct CachedGlyphInfo
			{
				TypesetData typeset;
				float line_y_offset;
				glm::vec2 alignment_position;
			};
			mutable std::vector<CachedGlyphInfo> cached_info;
			mutable glm::vec2 last_jitter_offset = {};

			mutable DirtyGlyphGroup dirty = ~DirtyGlyphGroup(0);

		public:
			GlyphGroup(TextElement&& element);

			void set_batch(SpriteBatch* batch);

			void draw() const;

			struct PeekData
			{
				utf::Codepoint first_codepoint = utf::Codepoint(0);
			};

			PeekData peek() const;
			void build_page_section(TypesetData& typeset, PeekData next_peek) const;

			enum class WriteResult
			{
				CONTINUE,
				BREAK
			};
			WriteResult write_glyph_section(TypesetData& typeset, PeekData next_peek) const;
			void clear_cache() const;

		private:
			bool can_fit_on_line(const TypesetData& typeset, float dx) const;
			bool can_fit_vertically(const TypesetData& typeset, float dy) const;

			void build_adj_offset(TypesetData& typeset, PeekData next_peek) const;
			void build_space(TypesetData& typeset, utf::Codepoint next_codepoint) const;
			void build_tab(TypesetData& typeset, utf::Codepoint next_codepoint) const;
			void build_newline(TypesetData& typeset) const;
			void build_glyph(TypesetData& typeset, float dx) const;

			struct LineAlignment
			{
				float y_offset;
			};

			bool write_adj_offset(TypesetData& typeset, PeekData next_peek, LineAlignment& line) const;
			void write_space(TypesetData& typeset, utf::Codepoint next_codepoint) const;
			void write_tab(TypesetData& typeset, utf::Codepoint next_codepoint) const;
			bool write_newline(TypesetData& typeset, LineAlignment& line) const;
			void write_glyph(TypesetData& typeset, utf::Codepoint c, float dx, LineAlignment line) const;

		public:
			float space_width(utf::Codepoint next_codepoint) const;
			float tab_width(utf::Codepoint next_codepoint) const;
			float advance_width(utf::Codepoint codepoint, utf::Codepoint next_codepoint) const;

		private:
			void recolor() const;
			void realign_lines() const;
			void reposition_jitter() const;

		public:
			void rewrite_alignment_positions() const;
		};
	}

	struct TextElementExposure
	{
	private:
		friend class Paragraph;
		friend class internal::GlyphGroup;
		Paragraph& paragraph;
		internal::GlyphGroup& glyph_group;

		TextElementExposure(Paragraph& paragraph, internal::GlyphGroup& glyph_group);

	public:
		void set_font(const FontAtlasRef& font);
		void set_text(utf::String&& text);
		void set_text_color(glm::vec4 color);
		void set_adj_offset(float adj_offset);
		void set_scale(glm::vec2 scale);
		void set_line_y_pivot(float line_y_pivot);
		void set_jitter_offset(glm::vec2 jitter_offset);
	};

	class Paragraph
	{
		friend class internal::GlyphGroup;
		friend struct ParagraphFormatExposure;
		friend struct TextElementExposure;

		mutable Sprite bkg;
		ParagraphFormat format;

		mutable internal::DirtyParagraph dirty_layout = internal::DirtyParagraph::REBUILD_LAYOUT;

		std::vector<internal::GlyphGroup> glyph_groups;

		mutable internal::PageBuildData page_data;
		mutable internal::PageLayout page_layout;
		mutable internal::AlignmentCache alignment_cache;
		mutable size_t written_glyph_groups = 0;

		mutable Transformer2D transformer;

	public:
		bool draw_bkg = false;

		Paragraph(std::vector<TextElement>&& elements, const ParagraphFormat& format = {});
		Paragraph(SpriteBatch* batch, std::vector<TextElement>&& elements, const ParagraphFormat& format = {});
		Paragraph(const Paragraph&);
		Paragraph(Paragraph&&) noexcept;
		Paragraph& operator=(const Paragraph&);
		Paragraph& operator=(Paragraph&&) noexcept;

		Transformer2DConstExposure get_transformer() const { return transformer; }
		Transformer2DExposure<TExposureParams{
			.local = exposure::local::FULL,
			.chain = exposure::chain::ATTACH_ONLY,
			.modifier = exposure::modifier::FULL
		}> set_transformer() { return transformer; }

	private:
		void init(std::vector<TextElement>&& elements);

	public:
		SpriteBatch* get_batch() const { return bkg.get_batch(); }
		void set_batch(SpriteBatch* batch);

		const ParagraphFormat& get_format() const { return format; }
		ParagraphFormatExposure set_format() { return ParagraphFormatExposure(*this); }
		glm::vec4 get_bkg_color() const;
		void set_bkg_color(glm::vec4 color);

		const TextElement& get_element(size_t i = 0) const { return glyph_groups[i].element; }
		TextElementExposure set_element(size_t i = 0) { return TextElementExposure(*this, glyph_groups[i]); }
		size_t get_element_count() const;
		void add_element(TextElement&& element);
		void insert_element(size_t i, TextElement&& element);
		void erase_element(size_t i);

		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { return transformer.set_local(); }

		glm::vec2 get_content_size() const { return page_layout.content_size; }
		glm::vec2 get_fitted_size() const { return page_layout.fitted_size; }

		void draw() const;

	private:
		void build_layout() const;
		void build_page() const;
		void write_glyphs() const;
		void compute_alignment_cache() const;
		void recompute_horizontal_alignment() const;
		void recompute_vertical_alignment() const;
		internal::GlyphGroup::PeekData peek_next(size_t i) const;

		void realign_horizontally() const;
		void realign_vertically() const;
	};

	typedef SmartReference<Paragraph> ParagraphRef;
}
