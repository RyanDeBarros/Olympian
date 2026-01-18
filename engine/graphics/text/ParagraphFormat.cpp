#include "ParagraphFormat.h"

#include "core/util/Logger.h"
#include "core/util/Loader.h"

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

		if (auto halign = node["horizontal_align"].value<std::string>())
		{
			const std::string& align = halign.value();
			if (align == "left")
				format.horizontal_alignment = rendering::ParagraphFormat::HorizontalAlignment::Left;
			else if (align == "center")
				format.horizontal_alignment = rendering::ParagraphFormat::HorizontalAlignment::Center;
			else if (align == "right")
				format.horizontal_alignment = rendering::ParagraphFormat::HorizontalAlignment::Right;
			else if (align == "justify")
				format.horizontal_alignment = rendering::ParagraphFormat::HorizontalAlignment::Justify;
			else if (align == "full_justify")
				format.horizontal_alignment = rendering::ParagraphFormat::HorizontalAlignment::FullJustify;
			else
				_OLY_ENGINE_LOG_WARNING("ASSETS") << "Unrecognized horizontal_alignment \"" << align << "\"." << LOG.nl;
		}

		if (auto valign = node["vertical_align"].value<std::string>())
		{
			const std::string& align = valign.value();
			if (align == "top")
				format.vertical_alignment = rendering::ParagraphFormat::VerticalAlignment::Top;
			else if (align == "middle")
				format.vertical_alignment = rendering::ParagraphFormat::VerticalAlignment::Middle;
			else if (align == "bottom")
				format.vertical_alignment = rendering::ParagraphFormat::VerticalAlignment::Bottom;
			else if (align == "justify")
				format.vertical_alignment = rendering::ParagraphFormat::VerticalAlignment::Justify;
			else if (align == "full_justify")
				format.vertical_alignment = rendering::ParagraphFormat::VerticalAlignment::FullJustify;
			else
				_OLY_ENGINE_LOG_WARNING("ASSETS") << "Unrecognized vertical_alignment \"" << align << "\"." << LOG.nl;
		}

		return format;
	}
}
