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
			unsigned int atlas_index = 0;
			utf::String text;
			Transform2D local;
			std::optional<bool> draw_bkg;
			std::optional<glm::vec4> bkg_color, text_color;
			std::vector<std::pair<int, glm::vec4>> glyph_colors;

			rendering::ParagraphFormat format;
		};
	}

	extern rendering::Paragraph load_paragraph(const TOMLNode& node);
	extern rendering::Paragraph load_paragraph(const params::Paragraph& params);
	extern rendering::Paragraph load_paragraph(params::Paragraph&& params);
}
