#include "TextElement.h"

#include "graphics/text/TextGlyph.h"
#include "core/algorithms/TaggedTextParser.h"
#include "core/algorithms/STLUtils.h"
#include "core/algorithms/Regex.h"

namespace oly::rendering
{
	float TextElement::line_height() const
	{
		return std::visit([](const auto& font) { return font->line_height(); }, font) * scale.y;
	}

	std::vector<TextElement> TextElement::expand(const TextElement& element)
	{
		std::vector<TextElement> expanded;
		expand(element, expanded);
		return expanded;
	}

	struct AttributeOverrides
	{
		bool font = false;
		bool text_color = false;
		bool adj_offset = false;
		bool scale = false;
		bool line_y_pivot = false;
		bool jitter_offset = false;

		bool all() const
		{
			return font && text_color && adj_offset && scale && line_y_pivot && jitter_offset;
		}
	};

	static std::string get_tag_field(const std::string& tag, size_t eq_pos)
	{
		return algo::to_lower(algo::trim(tag.substr(0, eq_pos)));
	}

	static std::string get_tag_value(const std::string& tag, size_t eq_pos)
	{
		return algo::trim(tag.substr(eq_pos + 1));
	}

	static void apply_tag(const std::string& tag, TextElement& e, AttributeOverrides& overrides)
	{
		size_t eq_pos = tag.find('=');
		if (eq_pos == std::string::npos)
			return;

		std::string field = get_tag_field(tag, eq_pos);

		if (field == "font")
		{
			if (!overrides.font)
			{
				std::string value = get_tag_value(tag, eq_pos);
				// TODO v5 load font from value - value should have the following syntax: "type:params", where to_lower(trim(type)) is "style" or "file" - if file, determine whether typeface or raster.
				overrides.font = true;
			}
		}
		else if (field == "color")
		{
			if (!overrides.text_color)
			{
				std::string value = get_tag_value(tag, eq_pos);
				if (algo::re::parse_vec4(value, e.text_color))
					overrides.text_color = true;
				else
				{
					algo::to_lower(value);
					if (value == "red")
					{
						e.text_color = { 1.0f, 0.0f, 0.0f, 1.0f };
						overrides.text_color = true;
					}
					else if (value == "green")
					{
						e.text_color = { 0.0f, 1.0f, 0.0f, 1.0f };
						overrides.text_color = true;
					}
					else if (value == "blue")
					{
						e.text_color = { 0.0f, 0.0f, 1.0f, 1.0f };
						overrides.text_color = true;
					}
					else if (value == "cyan")
					{
						e.text_color = { 0.0f, 1.0f, 1.0f, 1.0f };
						overrides.text_color = true;
					}
					else if (value == "magenta")
					{
						e.text_color = { 1.0f, 0.0f, 1.0f, 1.0f };
						overrides.text_color = true;
					}
					else if (value == "yellow")
					{
						e.text_color = { 1.0f, 1.0f, 0.0f, 1.0f };
						overrides.text_color = true;
					}
				}
			}
		}
		else if (field == "adj_offset")
		{
			if (!overrides.adj_offset)
			{
				if (algo::re::parse_float(get_tag_value(tag, eq_pos), e.adj_offset))
					overrides.adj_offset = true;
			}
		}
		else if (field == "scale")
		{
			if (!overrides.scale)
			{
				if (algo::re::parse_vec2(get_tag_value(tag, eq_pos), e.scale))
					overrides.scale = true;
			}
		}
		else if (field == "line_y_pivot")
		{
			if (!overrides.line_y_pivot)
			{
				float line_y_pivot;
				if (algo::re::parse_float(get_tag_value(tag, eq_pos), line_y_pivot))
				{
					overrides.line_y_pivot = true;
					e.line_y_pivot = line_y_pivot;
				}
			}
		}
		else if (field == "jitter_offset")
		{
			if (!overrides.jitter_offset)
			{
				if (algo::re::parse_vec2(get_tag_value(tag, eq_pos), e.jitter_offset))
					overrides.jitter_offset = true;
			}
		}
	}

	void TextElement::expand(const TextElement& element, std::vector<TextElement>& to)
	{
		algo::UTFTaggedTextParser parse(element.text);
		to.reserve(to.size() + parse.groups.size());
		for (algo::UTFTaggedTextParser::Group& group : parse.groups)
		{
			TextElement e{ .text = std::move(group.str) };

			AttributeOverrides overrides;

			while (!group.tags.empty())
			{
				utf::String tag = group.tags.top();
				group.tags.pop();
				apply_tag(tag.string(), e, overrides);
				if (overrides.all())
					break;
			}

			if (!overrides.font)
				e.font = element.font;
			if (!overrides.text_color)
				e.text_color = element.text_color;
			if (!overrides.adj_offset)
				e.adj_offset = element.adj_offset;
			if (!overrides.scale)
				e.scale = element.scale;
			if (!overrides.line_y_pivot)
				e.line_y_pivot = element.line_y_pivot;
			if (!overrides.jitter_offset)
				e.jitter_offset = element.jitter_offset;

			to.push_back(std::move(e));
		}
	}

	bool internal::font_equals(const TextElement& element, const FontAtlasRef& font)
	{
		return element.font.index() == TextElementFontIndex::ATLAS && std::get<TextElementFontIndex::ATLAS>(element.font) == font;
	}
	
	bool internal::font_equals(const TextElement& element, const RasterFontRef& font)
	{
		return element.font.index() == TextElementFontIndex::RASTER && std::get<TextElementFontIndex::RASTER>(element.font) == font;
	}

	bool internal::has_same_font_face(const TextElement& a, const TextElement& b)
	{
		if (a.font.index() != b.font.index())
			return false;

		switch (a.font.index())
		{
		case internal::TextElementFontIndex::ATLAS:
			return std::get<internal::TextElementFontIndex::ATLAS>(a.font)->font_face() == std::get<internal::TextElementFontIndex::ATLAS>(b.font)->font_face();
		case internal::TextElementFontIndex::RASTER:
			return std::get<internal::TextElementFontIndex::RASTER>(a.font) == std::get<internal::TextElementFontIndex::RASTER>(b.font);
		default:
			throw Error(ErrorCode::NOT_IMPLEMENTED);
		}
	}

	bool internal::support(const TextElement& element, utf::Codepoint c)
	{
		return std::visit([c](const auto& font) {
			if constexpr (visiting_class_is<decltype(font), FontAtlasRef>)
				return font->cache(c);
			else if constexpr (visiting_class_is<decltype(font), RasterFontRef>)
				return font->supports(c);
			else
				static_assert(deferred_false<decltype(font)>);
			}, element.font);
	}

	void internal::set_glyph(TextGlyph& glyph, const TextElement& element, utf::Codepoint c, glm::vec2 pos)
	{
		std::visit([&glyph, scale = element.scale, c, pos](const auto& font) {
			if constexpr (visiting_class_is<decltype(font), FontAtlasRef>)
				glyph.set_glyph(*font, font->get_glyph(c), pos, scale);
			else if constexpr (visiting_class_is<decltype(font), RasterFontRef>)
				glyph.set_glyph(*font, font->get_glyph(c), pos, scale);
			else
				static_assert(deferred_false<decltype(font)>);
			}, element.font);
	}

	float internal::advance_width(const TextElement& element, utf::Codepoint c, utf::Codepoint next_codepoint)
	{
		float adv = 0.0f;
		if (c != utf::Codepoint(' '))
			adv = std::visit([c](const auto& font) {
				if constexpr (visiting_class_is<decltype(font), FontAtlasRef>)
					return font->get_glyph(c).advance_width() * font->get_scale();
				else if constexpr (visiting_class_is<decltype(font), RasterFontRef>)
					return font->get_glyph(c).advance_width() * font->get_scale().x;
				else
					static_assert(deferred_false<decltype(font)>);
				}, element.font);
		else
			adv = std::visit([](const auto& font) { return font->get_scaled_space_advance_width(); }, element.font);

		if (next_codepoint)
			adv += std::visit([c, next_codepoint](const auto& font) { return font->kerning_of(c, next_codepoint); }, element.font);

		return adv * element.scale.x;
	}
}
