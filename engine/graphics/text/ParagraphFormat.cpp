#include "ParagraphFormat.h"

#include "core/util/Logger.h"
#include "core/util/Parser.h"

#include "definitions/Keys.h"

namespace oly::rendering
{
	bool ParagraphFormat::can_fit_on_line(TypesetData t, float dx) const
	{
		return text_wrap <= 0.0f || t.x + dx <= text_wrap;
	}

	bool ParagraphFormat::can_fit_vertically(TypesetData t, float dy) const
	{
		return max_height <= 0.0f || -t.y + dy <= max_height;
	}

	ParagraphFormat ParagraphFormat::load(TOMLNode node)
	{
		assets::Parser parser(node);

		ParagraphFormat format;

		parser.optional(detail::Key::Pivot)(format.pivot);
		parser.optional(detail::Key::LineSpacing)(format.line_spacing);
		parser.optional(detail::Key::LinebreakSpacing)(format.linebreak_spacing);
		parser.optional(detail::Key::MinSize)(format.min_size);
		parser.optional(detail::Key::Padding)(format.padding);
		parser.optional(detail::Key::TextWrap)(format.text_wrap);
		parser.optional(detail::Key::MaxHeight)(format.max_height);
		parser.optional(detail::Key::TabSpaces)(format.tab_spaces);
		parser.optional(detail::Key::HorizontalAlignment)(format.horizontal_alignment);
		parser.optional(detail::Key::VerticalAlignment)(format.vertical_alignment);

		return format;
	}
}
