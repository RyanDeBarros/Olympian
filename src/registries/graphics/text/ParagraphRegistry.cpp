#include "ParagraphRegistry.h"

#include "core/base/Context.h"
#include "registries/Loader.h"

namespace oly::reg
{
	void ParagraphRegistry::load(const char* registry_file)
	{
		auto toml = load_toml(registry_file);
		auto parageaph_list = toml["paragraph"].as_array();
		if (!parageaph_list)
			return;
		parageaph_list->for_each([this](auto&& node) {
			if constexpr (toml::is_table<decltype(node)>)
			{
				if (auto _name = node["name"].value<std::string>())
				{
					const std::string& name = _name.value();
					constructors[name] = node;
					if (auto _init = node["init"].value<std::string>())
					{
						auto_loaded.emplace(name, move_shared(create_paragraph(name)));
						if (_init.value() == "discard")
							constructors.erase(name);
					}
				}
			}
			});
	}

	void ParagraphRegistry::clear()
	{
		constructors.clear();
		auto_loaded.clear();
	}

	static rendering::ParagraphFormat create_format(const TOMLNode& node)
	{
		rendering::ParagraphFormat format;

		parse_vec2(node, "pivot", format.pivot);
		parse_float(node, "line spacing", format.line_spacing);
		parse_float(node, "linebreak spacing", format.linebreak_spacing);
		parse_vec2(node, "min size", format.min_size);
		parse_vec2(node, "padding", format.padding);
		parse_float(node, "text wrap", format.text_wrap);
		parse_float(node, "max height", format.max_height);

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
		
	rendering::Paragraph ParagraphRegistry::create_paragraph(const std::string& name) const
	{
		auto it = constructors.find(name);
		if (it == constructors.end())
			throw Error(ErrorCode::UNREGISTERED_PARAGRAPH);
		const auto& node = it->second;

		auto font_atlas = node["font atlas"].value<std::string>();
		if (!font_atlas)
			throw Error(ErrorCode::LOAD_ASSET);

		auto text = node["text"].value<std::string>().value_or("");

		rendering::Paragraph paragraph(context::text_batch(), context::ref_font_atlas(font_atlas.value()).lock(), create_format(node["format"]), std::move(text));
		if (auto draw_bkg = node["draw bkg"].value<bool>())
			paragraph.draw_bkg = draw_bkg.value();
		paragraph.set_local() = load_transform_2d(node, "transform");

		glm::vec4 v;
		if (parse_vec4(node, "bkg color", v))
			paragraph.set_bkg_color({ v });
		if (parse_vec4(node, "text color", paragraph.default_text_color.color))
			paragraph.recolor_text_with_default();

		auto glyph_colors = node["glyph colors"].as_table();
		if (glyph_colors)
		{
			for (const auto& [k, v] : *glyph_colors)
			{
				glm::vec4 gc;
				if (parse_vec4(v.as_array(), gc))
					paragraph.set_glyph_color(std::stoi(k.data()), { gc });
			}
		}

		return paragraph;
	}
		
	std::weak_ptr<rendering::Paragraph> ParagraphRegistry::ref_paragraph(const std::string& name) const
	{
		auto it = auto_loaded.find(name);
		if (it == auto_loaded.end())
			throw Error(ErrorCode::UNREGISTERED_PARAGRAPH);
		return it->second;
	}
		
	void ParagraphRegistry::delete_paragraph(const std::string& name)
	{
		auto_loaded.erase(name);
	}
}
