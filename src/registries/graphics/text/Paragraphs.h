#pragma once

#include "external/TOML.h"

#include "graphics/text/Paragraph.h"

namespace oly::reg
{
	namespace params
	{
		struct Paragraph
		{
			std::string font_atlas;
			std::string text; // TODO use utf::String ?
			Transform2D local;
			std::optional<bool> draw_bkg;
			std::optional<glm::vec4> bkg_color, text_color;
			std::vector<std::pair<int, glm::vec4>> glyph_colors;

			struct
			{
				std::optional<glm::vec2> pivot, min_size, padding;
				std::optional<float> line_spacing, linebreak_spacing, text_wrap, max_height;
				std::optional<rendering::ParagraphFormat::HorizontalAlignment> halign;
				std::optional<rendering::ParagraphFormat::VerticalAlignment> valign;
			} format;
		};
	}

	extern rendering::Paragraph load_paragraph(const TOMLNode& node);
	extern rendering::Paragraph load_paragraph(params::Paragraph&& params);
}
