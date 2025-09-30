#pragma once

#include "external/GLM.h"

#include "core/base/Parameters.h"
#include "core/util/UTF.h"

namespace oly::rendering
{
	namespace internal
	{
		enum DirtyParagraph
		{
			REBUILD_LAYOUT = 1 << 0,
			HORIZONTAL_ALIGN = 1 << 1,
			VERTICAL_ALIGN = 1 << 2,
			PADDING = 1 << 3,
			PIVOT = 1 << 4,
			LINE_SPACING = 1 << 5,
			MIN_SIZE = 1 << 6
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

	struct TextElementBase
	{
		utf::String text = "";
		glm::vec4 text_color = glm::vec4(1.0f);
		float adj_offset = 0.0f;
		glm::vec2 scale = glm::vec2(1.0f);
		BoundedFloat<0.0f, 1.0f> line_y_pivot = 0.0f;
		glm::vec2 jitter_offset = {};
	};
}
