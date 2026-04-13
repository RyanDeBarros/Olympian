#include "ParagraphFormat.h"

#include "core/util/Logger.h"
#include "core/util/Loader.h"

#include ".gen/keys/ParagraphFormat.inl"

#include ".gen/enums/rendering/text/HorizontalAlignment.inl"
#include ".gen/enums/rendering/text/VerticalAlignment.inl"

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
		ParagraphFormat format;

		io::try_parse_if_exists(node, _gen::keys::ParagraphFormat::Pivot, format.pivot);
		io::try_parse_if_exists(node, _gen::keys::ParagraphFormat::LineSpacing, format.line_spacing);
		io::try_parse_if_exists(node, _gen::keys::ParagraphFormat::LinebreakSpacing, format.linebreak_spacing);
		io::try_parse_if_exists(node, _gen::keys::ParagraphFormat::MinSize, format.min_size);
		io::try_parse_if_exists(node, _gen::keys::ParagraphFormat::Padding, format.padding);
		io::try_parse_if_exists(node, _gen::keys::ParagraphFormat::TextWrap, format.text_wrap);
		io::try_parse_if_exists(node, _gen::keys::ParagraphFormat::MaxHeight, format.max_height);
		io::try_parse_if_exists(node, _gen::keys::ParagraphFormat::TabSpaces, format.tab_spaces);
		io::try_parse_enum<_gen::rendering::text::HorizontalAlignment>(node, _gen::keys::ParagraphFormat::HorizontalAlignment, format.horizontal_alignment);
		io::try_parse_enum<_gen::rendering::text::VerticalAlignment>(node, _gen::keys::ParagraphFormat::VerticalAlignment, format.vertical_alignment);

		return format;
	}
}
