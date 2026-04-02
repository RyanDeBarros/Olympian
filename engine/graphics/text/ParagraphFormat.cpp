#include "ParagraphFormat.h"

#include "core/util/Logger.h"
#include "core/util/Loader.h"

#include ".gen/enums/text/HorizontalAlignment.inl"
#include ".gen/enums/text/VerticalAlignment.inl"

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

		io::parse_vec(node["pivot"], format.pivot);
		io::parse_float(node["line_spacing"], format.line_spacing);
		io::parse_float(node["linebreak_spacing"], format.linebreak_spacing);
		io::parse_vec(node["min_size"], format.min_size);
		io::parse_vec(node["padding"], format.padding);
		io::parse_float(node["text_wrap"], format.text_wrap);
		io::parse_float(node["max_height"], format.max_height);
		io::parse_float(node["tab_spaces"], format.tab_spaces);

		if (auto halign = io::parse_uint(node["horizontal_align"]))
		{
			try
			{
				format.horizontal_alignment = _gen::text::HorizontalAlignment::val(*halign);
			}
			catch (const std::out_of_range&)
			{
				_OLY_ENGINE_LOG_WARNING("ASSETS") << "Unrecognized \"horizontal_alignment\" (" << *halign << ")" << LOG.nl;
			}
		}

		if (auto valign = io::parse_uint(node["vertical_align"]))
		{
			try
			{
				format.vertical_alignment = _gen::text::VerticalAlignment::val(*valign);
			}
			catch (const std::out_of_range&)
			{
				_OLY_ENGINE_LOG_WARNING("ASSETS") << "Unrecognized \"vertical_alignment\" (" << *valign << ")" << LOG.nl;
			}
		}

		return format;
	}
}
