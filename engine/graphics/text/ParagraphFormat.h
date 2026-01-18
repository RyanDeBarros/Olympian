#pragma once

#include "external/GLM.h"
#include "external/TOML.h"

#include "core/base/Parameters.h"

namespace oly::rendering
{
	namespace internal
	{
		enum DirtyParagraph
		{
			RebuildLayout = 1 << 0,
			HorizontalAlign = 1 << 1,
			VerticalAlign = 1 << 2,
			Padding = 1 << 3,
			Pivot = 1 << 4,
			LineSpacing = 1 << 5,
			MinSize = 1 << 6
		};

		inline DirtyParagraph operator~(DirtyParagraph a) { return DirtyParagraph(~(int)a); }

		inline DirtyParagraph operator|(DirtyParagraph a, DirtyParagraph b) { return DirtyParagraph((int)a | (int)b); }
		inline DirtyParagraph& operator|=(DirtyParagraph& a, DirtyParagraph b) { a = a | b; return a; }

		inline DirtyParagraph operator&(DirtyParagraph a, DirtyParagraph b) { return DirtyParagraph((int)a & (int)b); }
		inline DirtyParagraph& operator&=(DirtyParagraph& a, DirtyParagraph b) { a = a & b; return a; }

		enum DirtyGlyphGroup
		{
			Recolor = 1 << 0,
			LineAlignment = 1 << 1,
			JitterOffset = 1 << 2
		};

		inline DirtyGlyphGroup operator~(DirtyGlyphGroup a) { return DirtyGlyphGroup(~(int)a); }

		inline DirtyGlyphGroup operator|(DirtyGlyphGroup a, DirtyGlyphGroup b) { return DirtyGlyphGroup((int)a | (int)b); }
		inline DirtyGlyphGroup& operator|=(DirtyGlyphGroup& a, DirtyGlyphGroup b) { a = a | b; return a; }

		inline DirtyGlyphGroup operator&(DirtyGlyphGroup a, DirtyGlyphGroup b) { return DirtyGlyphGroup((int)a & (int)b); }
		inline DirtyGlyphGroup& operator&=(DirtyGlyphGroup& a, DirtyGlyphGroup b) { a = a & b; return a; }
	}

	struct TypesetData
	{
		float x = 0.0f, y = 0.0f;
		glm::uint character = 0;
		size_t line = 0;
	};

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
			Left,
			Center,
			Right,
			Justify,
			FullJustify
		} horizontal_alignment = HorizontalAlignment::Left;

		enum class VerticalAlignment
		{
			Top,
			Middle,
			Bottom,
			Justify,
			FullJustify
		} vertical_alignment = VerticalAlignment::Top;

		bool can_fit_on_line(TypesetData t, float dx) const;
		bool can_fit_vertically(TypesetData t, float dy) const;

		static ParagraphFormat load(TOMLNode node);
	};
}
