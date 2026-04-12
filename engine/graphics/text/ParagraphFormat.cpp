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

		io::parse_vec(io::parse_key(node, _gen::keys::ParagraphFormat::Pivot), format.pivot);
		io::parse_float(io::parse_key(node, _gen::keys::ParagraphFormat::LineSpacing), format.line_spacing);
		io::parse_float(io::parse_key(node, _gen::keys::ParagraphFormat::LinebreakSpacing), format.linebreak_spacing);
		io::parse_vec(io::parse_key(node, _gen::keys::ParagraphFormat::MinSize), format.min_size);
		io::parse_vec(io::parse_key(node, _gen::keys::ParagraphFormat::Padding), format.padding);
		io::parse_float(io::parse_key(node, _gen::keys::ParagraphFormat::TextWrap), format.text_wrap);
		io::parse_float(io::parse_key(node, _gen::keys::ParagraphFormat::MaxHeight), format.max_height);
		io::parse_float(io::parse_key(node, _gen::keys::ParagraphFormat::TabSpaces), format.tab_spaces);

		if (auto halign = io::parse_uint(io::parse_key(node, _gen::keys::ParagraphFormat::HorizontalAlignment)))
		{
			try
			{
				format.horizontal_alignment = _gen::rendering::text::HorizontalAlignment::val(*halign);
			}
			catch (const std::out_of_range&)
			{
				_OLY_ENGINE_LOG_WARNING("ASSETS") << "Unrecognized " << io::key_string(_gen::keys::ParagraphFormat::HorizontalAlignment) << " (" << *halign << ")" << LOG.nl;
			}
		}

		if (auto valign = io::parse_uint(io::parse_key(node, _gen::keys::ParagraphFormat::VerticalAlignment)))
		{
			try
			{
				format.vertical_alignment = _gen::rendering::text::VerticalAlignment::val(*valign);
			}
			catch (const std::out_of_range&)
			{
				_OLY_ENGINE_LOG_WARNING("ASSETS") << "Unrecognized " << io::key_string(_gen::keys::ParagraphFormat::VerticalAlignment) << " (" << *valign << ")" << LOG.nl;
			}
		}

		return format;
	}
}
