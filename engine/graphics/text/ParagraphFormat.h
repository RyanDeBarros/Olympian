#pragma once

#include "core/base/Parameters.h"

#include "external/GLM.h"
#include "external/TOML.h"

#include <imp/bitmask.hpp>

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

        IMP_IMPLEMENT_BITMASK(DirtyParagraph);

		enum DirtyGlyphGroup
		{
			Recolor = 1 << 0,
			LineAlignment = 1 << 1,
			JitterOffset = 1 << 2
		};

        IMP_IMPLEMENT_BITMASK(DirtyGlyphGroup);
	}

    enum class HorizontalAlignment
    {
        Left,
        Center,
        Right,
        Justify,
        FullJustify
    };

    enum class VerticalAlignment
    {
        Top,
        Middle,
        Bottom,
        Justify,
        FullJustify
    };

	struct TypesetData
	{
        float x = 0.f;
        float y = 0.f;
		glm::uint character = 0;
		size_t line = 0;
	};

	struct ParagraphFormat
	{
		float line_spacing = 1.f;
		float tab_spaces = 4.f;
		float linebreak_spacing = 1.f;
		glm::vec2 pivot = { 0.5f, 0.5f };
		glm::vec2 min_size = {};
		float text_wrap = 0.f;
		float max_height = 0.f;
		glm::vec2 padding = {};
        HorizontalAlignment horizontal_alignment = HorizontalAlignment::Left;
        VerticalAlignment vertical_alignment = VerticalAlignment::Top;

		static ParagraphFormat load(TOMLNode node);

        bool can_fit_on_line(TypesetData t, float dx) const;
		bool can_fit_vertically(TypesetData t, float dy) const;
	};
}
