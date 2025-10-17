#include "Paragraphs.h"

#include "core/context/rendering/Fonts.h"
#include "registries/Loader.h"

// TODO v5 devise a better design for asset loading. Right now, essentially the same parsing is done three times: 1. convert TOML to params. 2. convert params to actual object. 3. in editor archetype prebuild, convert TOML to params.

namespace oly::reg
{
	static rendering::ParagraphFormat create_format(const TOMLNode& node)
	{
		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Parsing paragraph format [" << (src ? *src : "") << "]." << LOG.nl;
		}

		rendering::ParagraphFormat format;

		glm::vec2 v2;
		float v1;

		if (parse_vec(node["pivot"], v2))
			format.pivot = v2;
		if (parse_float(node["line_spacing"], v1))
			format.line_spacing = v1;
		if (parse_float(node["linebreak_spacing"], v1))
			format.linebreak_spacing = v1;
		if (parse_vec(node["min_size"], v2))
			format.min_size = v2;
		if (parse_vec(node["padding"], v2))
			format.padding = v2;
		if (parse_float(node["text_wrap"], v1))
			format.text_wrap = v1;
		if (parse_float(node["max_height"], v1))
			format.max_height = v1;
		if (parse_float(node["tab_spaces"], v1))
			format.tab_spaces = v1;

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

		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Paragraph format [" << (src ? *src : "") << "] parsed." << LOG.nl;
		}

		return format;
	}

	static std::optional<params::Paragraph::TextElement> parse_text_element(const TOMLNode& element, size_t i)
	{
		params::Paragraph::TextElement element_params;
		if (auto font_atlas = element["font_atlas"].value<std::string>())
			element_params.font_atlas = *font_atlas;
		else
		{
			OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Missing or invalid \"font_atlas\" string field in text element (" << i << ")." << LOG.nl;
			return std::nullopt;
		}

		parse_uint(element["atlas_index"], element_params.atlas_index);

		if (auto text = element["text"].value<std::string>())
			element_params.text = *text;
		else
		{
			OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Missing or invalid \"text\" string field in text element (" << i << ")." << LOG.nl;
			return std::nullopt;
		}

		glm::vec4 v;
		if (parse_vec(element["text_color"], v))
			element_params.text_color = v;

		parse_float(element["adj_offset"], element_params.adj_offset);
		parse_vec(element["scale"], element_params.scale);
		parse_float(element["line_y_pivot"], element_params.line_y_pivot);
		parse_vec(element["jitter_offset"], element_params.jitter_offset);
		parse_bool(element["expand"], element_params.expand);

		return element_params;
	}

	rendering::Paragraph load_paragraph(const TOMLNode& node)
	{
		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Parsing paragraph [" << (src ? *src : "") << "]." << LOG.nl;
		}

		params::Paragraph params;

		if (auto element_array = node["element"].as_array())
		{
			for (size_t i = 0; i < element_array->size(); ++i)
			{
				if (auto element = TOMLNode(*element_array->get(i)))
				{
					std::optional<params::Paragraph::TextElement> element_params = parse_text_element(element, i);
					if (element_params.has_value())
						params.elements.emplace_back(std::move(*element_params));
				}
			}
		}
		else if (auto element = node["element"])
		{
			std::optional<params::Paragraph::TextElement> element_params = parse_text_element(element, 0);
			if (element_params.has_value())
				params.elements = { std::move(*element_params) };
		}

		params.format = create_format(node["format"]);
		parse_bool(node["draw_bkg"], params.draw_bkg);
		params.local = load_transform_2d(node["transform"]);

		glm::vec4 v;
		if (parse_vec(node["bkg_color"], v))
			params.bkg_color = v;

		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Paragraph [" << (src ? *src : "") << "] parsed." << LOG.nl;
		}

		return load_paragraph(std::move(params));
	}

	static void add_element(std::vector<rendering::TextElement>& elements, const params::Paragraph::TextElement& params)
	{
		rendering::TextElement element{
			.font = context::load_font_atlas(params.font_atlas, params.atlas_index), // TODO v5 support for raster font
			.text = params.text,
			.adj_offset = params.adj_offset,
			.scale = params.scale,
			.line_y_pivot = params.line_y_pivot,
			.jitter_offset = params.jitter_offset
		};
		
		if (params.text_color)
			element.text_color = *params.text_color;

		if (params.expand)
			rendering::TextElement::expand(element, elements);
		else
			elements.push_back(std::move(element));
	}

	static void add_element(std::vector<rendering::TextElement>& elements, params::Paragraph::TextElement&& params)
	{
		rendering::TextElement element{
			.font = context::load_font_atlas(params.font_atlas, params.atlas_index), // TODO v5 support for raster font
			.text = std::move(params.text),
			.adj_offset = params.adj_offset,
			.scale = params.scale,
			.line_y_pivot = params.line_y_pivot,
			.jitter_offset = params.jitter_offset
		};

		if (params.text_color)
			element.text_color = *params.text_color;

		if (params.expand)
			rendering::TextElement::expand(element, elements);
		else
			elements.push_back(std::move(element));
	}

	rendering::Paragraph load_paragraph(const params::Paragraph& params)
	{
		std::vector<rendering::TextElement> elements;
		for (const params::Paragraph::TextElement& pelement : params.elements)
			add_element(elements, pelement);

		rendering::Paragraph paragraph(std::move(elements), params.format);

		paragraph.set_local() = params.local;
		paragraph.draw_bkg = params.draw_bkg;
		if (params.bkg_color)
			paragraph.set_bkg_color(*params.bkg_color);

		return paragraph;
	}

	rendering::Paragraph load_paragraph(params::Paragraph&& params)
	{
		std::vector<rendering::TextElement> elements;
		for (params::Paragraph::TextElement& pelement : params.elements)
			add_element(elements, std::move(pelement));

		rendering::Paragraph paragraph(std::move(elements), params.format);

		paragraph.set_local() = params.local;
		paragraph.draw_bkg = params.draw_bkg;
		if (params.bkg_color)
			paragraph.set_bkg_color(*params.bkg_color);

		return paragraph;
	}
}
