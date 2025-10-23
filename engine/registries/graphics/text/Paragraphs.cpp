#include "Paragraphs.h"

#include "core/context/rendering/Fonts.h"
#include "registries/Loader.h"

// TODO v5 devise a better design for asset loading. Right now, essentially the same parsing is done three times: 1. convert TOML to params. 2. convert params to actual object. 3. in editor archetype prebuild, convert TOML to params.

namespace oly::reg
{
	static rendering::ParagraphFormat load_format(TOMLNode node)
	{
		rendering::ParagraphFormat format;

		parse_vec(node["pivot"], format.pivot);
		parse_float(node["line_spacing"], format.line_spacing);
		parse_float(node["linebreak_spacing"], format.linebreak_spacing);
		parse_vec(node["min_size"], format.min_size);
		parse_vec(node["padding"], format.padding);
		parse_float(node["text_wrap"], format.text_wrap);
		parse_float(node["max_height"], format.max_height);
		parse_float(node["tab_spaces"], format.tab_spaces);

		if (auto halign = node["horizontal_align"].value<std::string>())
		{
			const std::string& align = halign.value();
			if (align == "left")
				format.horizontal_alignment = rendering::ParagraphFormat::HorizontalAlignment::LEFT;
			else if (align == "center")
				format.horizontal_alignment = rendering::ParagraphFormat::HorizontalAlignment::CENTER;
			else if (align == "right")
				format.horizontal_alignment = rendering::ParagraphFormat::HorizontalAlignment::RIGHT;
			else if (align == "justify")
				format.horizontal_alignment = rendering::ParagraphFormat::HorizontalAlignment::JUSTIFY;
			else if (align == "full_justify")
				format.horizontal_alignment = rendering::ParagraphFormat::HorizontalAlignment::FULL_JUSTIFY;
			else
				OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Unrecognized horizontal_alignment \"" << align << "\"." << LOG.nl;
		}

		if (auto valign = node["vertical_align"].value<std::string>())
		{
			const std::string& align = valign.value();
			if (align == "top")
				format.vertical_alignment = rendering::ParagraphFormat::VerticalAlignment::TOP;
			else if (align == "middle")
				format.vertical_alignment = rendering::ParagraphFormat::VerticalAlignment::MIDDLE;
			else if (align == "bottom")
				format.vertical_alignment = rendering::ParagraphFormat::VerticalAlignment::BOTTOM;
			else if (align == "justify")
				format.vertical_alignment = rendering::ParagraphFormat::VerticalAlignment::JUSTIFY;
			else if (align == "full_justify")
				format.vertical_alignment = rendering::ParagraphFormat::VerticalAlignment::FULL_JUSTIFY;
			else
				OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Unrecognized vertical_alignment \"" << align << "\"." << LOG.nl;
		}

		return format;
	}

	static void add_text_element(TOMLNode element, size_t i, std::vector<rendering::TextElement>& elements)
	{
		rendering::TextElement e;
		if (auto font = element["font"].value<std::string>())
			e.font = context::load_font(*font, parse_uint_or(element["font_index"], 0));
		else
		{
			OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Missing or invalid \"font_atlas\" string field in text element (" << i << ")." << LOG.nl;
			return;
		}

		if (auto text = element["text"].value<std::string>())
			e.text = std::move(*text);
		else
		{
			OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Missing or invalid \"text\" string field in text element (" << i << ")." << LOG.nl;
			return;
		}

		parse_vec(element["text_color"], e.text_color);
		parse_float(element["adj_offset"], e.adj_offset);
		parse_vec(element["scale"], e.scale);
		float line_y_pivot;
		if (parse_float(element["line_y_pivot"], line_y_pivot))
			e.line_y_pivot = line_y_pivot;
		parse_vec(element["jitter_offset"], e.jitter_offset);

		if (parse_bool_or(element["expand"], false))
			rendering::TextElement::expand(e, elements);
		else
			elements.push_back(std::move(e));
	}

	rendering::Paragraph load_paragraph(TOMLNode node)
	{
		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Parsing paragraph [" << (src ? *src : "") << "]..." << LOG.nl;
		}

		std::vector<rendering::TextElement> elements;
		if (auto element_array = node["element"].as_array())
		{
			for (size_t i = 0; i < element_array->size(); ++i)
				if (auto element = TOMLNode(*element_array->get(i)))
					add_text_element(element, i, elements);
		}
		else if (auto element = node["element"])
			add_text_element(element, 0, elements);

		rendering::Paragraph paragraph(std::move(elements), load_format(node["format"]));
		if (auto transformer = node["transformer"])
		{
			paragraph.set_local() = load_transform_2d(transformer);
			paragraph.set_transformer().set_modifier() = load_transform_modifier_2d(transformer["modifier"]);
		}

		parse_bool(node["draw_bkg"], paragraph.draw_bkg);
		glm::vec4 bkg_color;
		if (parse_vec(node["bkg_color"], bkg_color))
			paragraph.set_bkg_color(bkg_color);

		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "...Paragraph [" << (src ? *src : "") << "] parsed." << LOG.nl;
		}

		return paragraph;
	}
}
