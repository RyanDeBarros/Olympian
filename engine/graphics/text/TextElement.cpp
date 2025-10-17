#include "TextElement.h"

#include "TextGlyph.h"

namespace oly::rendering
{
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

	float TextElement::line_height() const
	{
		return std::visit([](const auto& font) { return font->line_height(); }, font) * scale.y;
	}
}
