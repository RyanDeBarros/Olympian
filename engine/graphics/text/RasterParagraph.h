#pragma once

#include "graphics/text/Text.h"
#include "graphics/text/RasterFont.h"
#include "graphics/text/Typesetting.h"
#include "graphics/sprites/Sprite.h"

#include "core/base/TransformerExposure.h"

namespace oly::rendering
{
	class RasterParagraph;

	struct RasterParagraphFormatExposure
	{
	private:
		friend class RasterParagraph;
		RasterParagraph& paragraph;

		RasterParagraphFormatExposure(RasterParagraph& paragraph);

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

	struct RasterTextElement
	{
		RasterFontRef font;
		TextElementBase base;

		float line_height() const { return font->line_height() * base.scale.y; }
	};

	class RasterTextElementExposure;

	namespace internal
	{
		class RasterGlyphGroup
		{
			friend class RasterParagraph;
			friend class RasterTextElementExposure;
			const RasterParagraph* paragraph = nullptr;
			RasterTextElement element;

			mutable std::vector<TextGlyph> glyphs;

			struct CachedGlyphInfo
			{
				TypesetData typeset;
				float line_y_offset;
			};
			mutable std::vector<CachedGlyphInfo> cached_info;
			mutable glm::vec2 last_jitter_offset = {};

			mutable DirtyGlyphGroup dirty = ~DirtyGlyphGroup(0);

		public:
			RasterGlyphGroup(RasterTextElement&& element);

			void set_batch(Unbatched);
			void set_batch(rendering::SpriteBatch& batch);

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
			glm::vec2 get_glyph_position(size_t i) const;

		public:
			float space_width(utf::Codepoint next_codepoint) const;
			float tab_width(utf::Codepoint next_codepoint) const;
			float advance_width(utf::Codepoint codepoint, utf::Codepoint next_codepoint) const;

		private:
			void recolor() const;
			void realign_lines() const;
			void reposition_jitter() const;

		public:
			void reposition_glyphs() const;
		};
	}

	struct RasterTextElementExposure
	{
	private:
		friend class RasterParagraph;
		friend class internal::RasterGlyphGroup;
		RasterParagraph& paragraph;
		internal::RasterGlyphGroup& glyph_group;

		RasterTextElementExposure(RasterParagraph& paragraph, internal::RasterGlyphGroup& glyph_group);

	public:
		void set_font(const RasterFontRef& font);
		void set_text(utf::String&& text);
		void set_text_color(glm::vec4 color);
		void set_adj_offset(float adj_offset);
		void set_scale(glm::vec2 scale);
		void set_line_y_pivot(float line_y_pivot);
		void set_jitter_offset(glm::vec2 jitter_offset);
	};

	class RasterParagraph
	{
		friend class internal::RasterGlyphGroup;
		friend struct RasterParagraphFormatExposure;
		friend struct RasterTextElementExposure;

		mutable Sprite bkg;
		ParagraphFormat format;

		mutable internal::DirtyParagraph dirty_layout = internal::DirtyParagraph::REBUILD_LAYOUT;

		std::vector<internal::RasterGlyphGroup> glyph_groups;

		mutable internal::PageBuildData page_data;
		mutable internal::PageLayout page_layout;
		mutable internal::AlignmentCache alignment_cache;
		mutable size_t written_glyph_groups = 0;

		mutable Transformer2D transformer;

	public:
		bool draw_bkg = false;

		RasterParagraph(std::vector<RasterTextElement>&& elements, const ParagraphFormat& format = {});
		RasterParagraph(Unbatched, std::vector<RasterTextElement>&& elements, const ParagraphFormat& format = {});
		RasterParagraph(SpriteBatch& batch, std::vector<RasterTextElement>&& elements, const ParagraphFormat& format = {});
		RasterParagraph(const RasterParagraph&);
		RasterParagraph(RasterParagraph&&) noexcept;
		RasterParagraph& operator=(const RasterParagraph&);
		RasterParagraph& operator=(RasterParagraph&&) noexcept;

		Transformer2DConstExposure get_transformer() const { return transformer; }
		Transformer2DExposure < TExposureParams{
			.local = exposure::local::FULL,
			.chain = exposure::chain::ATTACH_ONLY,
			.modifier = exposure::modifier::FULL
		} > set_transformer() { return transformer; }

	private:
		void init(std::vector<RasterTextElement>&& elements);

	public:
		auto get_batch() const { return bkg.get_batch(); }
		void set_batch(Unbatched);
		void set_batch(SpriteBatch& batch);

		const ParagraphFormat& get_format() const { return format; }
		RasterParagraphFormatExposure set_format() { return RasterParagraphFormatExposure(*this); }
		glm::vec4 get_bkg_color() const;
		void set_bkg_color(glm::vec4 color);

		const RasterTextElement& get_element(size_t i = 0) const { return glyph_groups[i].element; }
		RasterTextElementExposure set_element(size_t i = 0) { return RasterTextElementExposure(*this, glyph_groups[i]); }
		size_t get_element_count() const;
		void add_element(RasterTextElement&& element);
		void insert_element(size_t i, RasterTextElement&& element);
		void erase_element(size_t i);

		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { return transformer.set_local(); }

		glm::vec2 get_content_size() const { return page_layout.content_size; }
		glm::vec2 get_fitted_size() const { return page_layout.fitted_size; }

		void draw() const;

	private:
		void clean_dirty_layout() const;
		void build_layout() const;
		void write_glyphs() const;

		enum AlignmentFlags
		{
			RESIZE_LINES = 1 << 0,
			VERTICAL = 1 << 1,
			HORIZONTAL = 1 << 2,
			PIVOT = 1 << 3,
			PADDING = 1 << 4
		};
		void compute_alignment_cache(AlignmentFlags flags) const;

		internal::RasterGlyphGroup::PeekData peek_next(size_t i) const;
		void recompute_content_size_x() const;
		void recompute_content_size_y() const;
		void recompute_fitted_size_x() const;
		void recompute_fitted_size_y() const;
	};

	typedef SmartReference<RasterParagraph> RasterParagraphRef;
}
