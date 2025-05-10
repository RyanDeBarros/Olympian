#include "Paragraphs.h"

#include "core/base/Context.h"
#include "registries/Loader.h"

namespace oly::reg
{
	static void create_format(const TOMLNode& node, params::Paragraph& params)
	{
		auto& fparams = params.format;

		glm::vec2 v2;
		float v1;

		if (parse_vec2(node, "pivot", v2))
			fparams.pivot = v2;
		if (parse_float(node, "line spacing", v1))
			fparams.line_spacing = v1;
		if (parse_float(node, "linebreak spacing", v1))
			fparams.linebreak_spacing = v1;
		if (parse_vec2(node, "min size", v2))
			fparams.min_size = v2;
		if (parse_vec2(node, "padding", v2))
			fparams.padding = v2;
		if (parse_float(node, "text wrap", v1))
			fparams.text_wrap = v1;
		if (parse_float(node, "max height", v1))
			fparams.max_height = v1;

		if (auto halign = node["horizontal align"].value<std::string>())
		{
			const std::string& align = halign.value();
			if (align == "left")
				fparams.halign= rendering::ParagraphFormat::HorizontalAlignment::LEFT;
			else if (align == "center")
				fparams.halign = rendering::ParagraphFormat::HorizontalAlignment::CENTER;
			else if (align == "right")
				fparams.halign = rendering::ParagraphFormat::HorizontalAlignment::RIGHT;
			else if (align == "justify")
				fparams.halign = rendering::ParagraphFormat::HorizontalAlignment::JUSTIFY;
			else if (align == "full justify")
				fparams.halign = rendering::ParagraphFormat::HorizontalAlignment::FULL_JUSTIFY;
		}

		if (auto valign = node["vertical align"].value<std::string>())
		{
			const std::string& align = valign.value();
			if (align == "top")
				fparams.valign = rendering::ParagraphFormat::VerticalAlignment::TOP;
			else if (align == "middle")
				fparams.valign = rendering::ParagraphFormat::VerticalAlignment::MIDDLE;
			else if (align == "bottom")
				fparams.valign = rendering::ParagraphFormat::VerticalAlignment::BOTTOM;
			else if (align == "justify")
				fparams.valign = rendering::ParagraphFormat::VerticalAlignment::JUSTIFY;
			else if (align == "full justify")
				fparams.valign = rendering::ParagraphFormat::VerticalAlignment::FULL_JUSTIFY;
		}
	}

	static rendering::ParagraphFormat create_format(const params::Paragraph& params)
	{
		const auto& fparams = params.format;

		rendering::ParagraphFormat format;

		if (fparams.pivot)
			format.pivot = fparams.pivot.value();
		if (fparams.line_spacing)
			format.line_spacing = fparams.line_spacing.value();
		if (fparams.linebreak_spacing)
			format.linebreak_spacing = fparams.linebreak_spacing.value();
		if (fparams.min_size)
			format.min_size = fparams.min_size.value();
		if (fparams.padding)
			format.padding = fparams.padding.value();
		if (fparams.text_wrap)
			format.text_wrap = fparams.text_wrap.value();
		if (fparams.max_height)
			format.max_height = fparams.max_height.value();
		if (fparams.halign)
			format.horizontal_alignment = fparams.halign.value();
		if (fparams.valign)
			format.vertical_alignment = fparams.valign.value();

		return format;
	}
	
	rendering::Paragraph load_paragraph(const TOMLNode& node)
	{
		params::Paragraph params;

		auto font_atlas = node["font atlas"].value<std::string>();
		if (!font_atlas)
			throw Error(ErrorCode::LOAD_ASSET);

		params.font_atlas = font_atlas.value();
		create_format(node["format"], params);
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

	rendering::Paragraph load_paragraph(params::Paragraph&& params)
	{
		rendering::Paragraph paragraph = context::paragraph(params.font_atlas, create_format(params), std::move(params.text));
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
