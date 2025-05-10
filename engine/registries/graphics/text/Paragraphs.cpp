#include "Paragraphs.h"

#include "core/base/Context.h"
#include "registries/Loader.h"

namespace oly::reg
{
	static rendering::ParagraphFormat create_format(const TOMLNode& node)
	{
		rendering::ParagraphFormat format;

		glm::vec2 v2;
		float v1;

		if (parse_vec2(node, "pivot", v2))
			format.pivot = v2;
		if (parse_float(node, "line spacing", v1))
			format.line_spacing = v1;
		if (parse_float(node, "linebreak spacing", v1))
			format.linebreak_spacing = v1;
		if (parse_vec2(node, "min size", v2))
			format.min_size = v2;
		if (parse_vec2(node, "padding", v2))
			format.padding = v2;
		if (parse_float(node, "text wrap", v1))
			format.text_wrap = v1;
		if (parse_float(node, "max height", v1))
			format.max_height = v1;
		if (parse_float(node, "tab spaces", v1))
			format.tab_spaces = v1;

		if (auto halign = node["horizontal align"].value<std::string>())
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
			else if (align == "full justify")
				format.horizontal_alignment = rendering::ParagraphFormat::HorizontalAlignment::FULL_JUSTIFY;
		}

		if (auto valign = node["vertical align"].value<std::string>())
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
			else if (align == "full justify")
				format.vertical_alignment = rendering::ParagraphFormat::VerticalAlignment::FULL_JUSTIFY;
		}

		return format;
	}

	rendering::Paragraph load_paragraph(const TOMLNode& node)
	{
		params::Paragraph params;

		auto font_atlas = node["font atlas"].value<std::string>();
		if (!font_atlas)
			throw Error(ErrorCode::LOAD_ASSET);

		params.font_atlas = font_atlas.value();
		params.format = create_format(node["format"]);
		params.text = node["text"].value<std::string>().value_or("");

		if (auto draw_bkg = node["draw bkg"].value<bool>())
			params.draw_bkg = draw_bkg.value();
		params.local = load_transform_2d(node, "transform");

		glm::vec4 v;
		if (parse_vec4(node, "bkg color", v))
			params.bkg_color = v;
		if (parse_vec4(node, "text color", v))
			params.text_color = v;

		auto glyph_colors = node["glyph colors"].as_table();
		if (glyph_colors)
		{
			for (const auto& [k, v] : *glyph_colors)
			{
				glm::vec4 gc;
				if (parse_vec4(v.as_array(), gc))
					params.glyph_colors.push_back({ std::stoi(k.data()), { gc } });
			}
		}

		return load_paragraph(std::move(params));
	}

	rendering::Paragraph load_paragraph(const params::Paragraph& params)
	{
		rendering::Paragraph paragraph = context::paragraph(params.font_atlas, params.format, dupl(params.text));
		if (params.draw_bkg)
			paragraph.draw_bkg = params.draw_bkg.value();
		paragraph.set_local() = params.local;

		if (params.bkg_color)
			paragraph.set_bkg_color({ params.bkg_color.value() });
		if (params.text_color)
		{
			paragraph.default_text_color.color = params.text_color.value();
			paragraph.recolor_text_with_default();
		}

		for (const auto& gc : params.glyph_colors)
			paragraph.set_glyph_color(gc.first, { gc.second });

		return paragraph;
	}

	rendering::Paragraph load_paragraph(params::Paragraph&& params)
	{
		rendering::Paragraph paragraph = context::paragraph(params.font_atlas, params.format, std::move(params.text));
		if (params.draw_bkg)
			paragraph.draw_bkg = params.draw_bkg.value();
		paragraph.set_local() = params.local;

		if (params.bkg_color)
			paragraph.set_bkg_color({ params.bkg_color.value() });
		if (params.text_color)
		{
			paragraph.default_text_color.color = params.text_color.value();
			paragraph.recolor_text_with_default();
		}

		for (const auto& gc : params.glyph_colors)
			paragraph.set_glyph_color(gc.first, { gc.second });

		return paragraph;
	}
}
