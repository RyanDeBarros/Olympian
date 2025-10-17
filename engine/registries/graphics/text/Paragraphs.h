#pragma once

#include "external/TOML.h"

#include "graphics/text/Paragraph.h"

namespace oly::reg
{
	namespace params
	{
		struct Paragraph
		{
			struct TextElement
			{
				std::string font_atlas;
				unsigned int atlas_index = 0;
				utf::String text;
				std::optional<glm::vec4> text_color;
				float adj_offset = 0.0f;
				glm::vec2 scale = glm::vec2(1.0f);
				float line_y_pivot = 0.0f;
				glm::vec2 jitter_offset = {};
				bool expand = false;
			};

			std::vector<TextElement> elements;
			Transform2D local;
			std::optional<bool> draw_bkg;
			std::optional<glm::vec4> bkg_color;

			rendering::ParagraphFormat format;
		};
	}

	extern rendering::Paragraph load_paragraph(const TOMLNode& node);
	extern rendering::Paragraph load_paragraph(const params::Paragraph& params);
	extern rendering::Paragraph load_paragraph(params::Paragraph&& params);
}
