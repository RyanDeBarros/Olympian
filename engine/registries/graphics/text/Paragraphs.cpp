#include "Paragraphs.h"

#include "core/context/rendering/Text.h"
#include "registries/Loader.h"

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

		if (parse_vec(node["pivot"].as_array(), v2))
			format.pivot = v2;
		if (parse_float(node, "line spacing", v1))
			format.line_spacing = v1;
		if (parse_float(node, "linebreak spacing", v1))
			format.linebreak_spacing = v1;
		if (parse_vec(node["min size"].as_array(), v2))
			format.min_size = v2;
		if (parse_vec(node["padding"].as_array(), v2))
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
			else
				OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Unrecognized horizontal alignment \"" << align << "\"." << LOG.nl;
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
			else
				OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Unrecognized vertical alignment \"" << align << "\"." << LOG.nl;
		}

		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Paragraph format [" << (src ? *src : "") << "] parsed." << LOG.nl;
		}

		return format;
	}

	rendering::Paragraph load_paragraph(const TOMLNode& node)
	{
		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Parsing paragraph [" << (src ? *src : "") << "]." << LOG.nl;
		}

		params::Paragraph params;

		auto font_atlas = node["font_atlas"].value<std::string>();
		if (!font_atlas)
			throw Error(ErrorCode::LOAD_ASSET);

		params.font_atlas = font_atlas.value();
		if (auto index = node["atlas_index"].value<int64_t>())
			params.atlas_index = (unsigned int)index.value();
		params.format = create_format(node["format"]);
		params.text = node["text"].value<std::string>().value_or("");

		if (auto draw_bkg = node["draw_bkg"].value<bool>())
			params.draw_bkg = draw_bkg.value();
		params.local = load_transform_2d(node, "transform");

		glm::vec4 v;
		if (parse_vec(node["bkg_color"].as_array(), v))
			params.bkg_color = v;
		if (parse_vec(node["text_color"].as_array(), v))
			params.text_color = v;

		auto glyph_colors = node["glyph_colors"].as_table();
		if (glyph_colors)
		{
			for (const auto& [k, v] : *glyph_colors)
			{
				glm::vec4 gc;
				if (parse_vec(v.as_array(), gc))
					params.glyph_colors.push_back({ std::stoi(k.data()), { gc } });
				else
					OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Unrecognized glyph color for glyph #" << k.data() << "." << LOG.nl;
			}
		}

		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Paragraph [" << (src ? *src : "") << "] parsed." << LOG.nl;
		}

		return load_paragraph(std::move(params));
	}

	rendering::Paragraph load_paragraph(const params::Paragraph& params)
	{
		rendering::Paragraph paragraph = context::paragraph(params.font_atlas, params.format, dupl(params.text), params.atlas_index);
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
		rendering::Paragraph paragraph = context::paragraph(params.font_atlas, params.format, std::move(params.text), params.atlas_index);
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
