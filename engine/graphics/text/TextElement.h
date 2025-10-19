#pragma once

#include "core/base/Parameters.h"
#include "core/util/UTF.h"

#include "graphics/text/FontFamily.h"

namespace oly::rendering
{
	class TextGlyph;
	
	struct Font
	{
		// TODO v5 something more efficient than variant? same for FontFamily::FontRef
		using Variant = std::variant<FontAtlasRef, RasterFontRef, FontSelection>;

		Variant f;

		Font() = default;
		Font(const FontAtlasRef& font) : f(font) {}
		Font(const RasterFontRef& font) : f(font) {}
		Font(const FontSelection& font) : f(font) {}
		Font& operator=(const FontAtlasRef& font) { f = font; return *this; }
		Font& operator=(const RasterFontRef& font) { f = font; return *this; }
		Font& operator=(const FontSelection& font) { f = font; return *this; }

		bool try_apply_style(FontStyle style);
		bool try_unapply_style(FontStyle style);

		float line_height() const;

		bool operator==(const FontAtlasRef& font) const;
		bool operator==(const RasterFontRef& font) const;
		bool operator==(const FontSelection& font) const;
		bool operator==(const Font& other) const;

		bool adj_compat(const Font& other) const;
		bool support(utf::Codepoint c) const;
		void set_glyph(TextGlyph& glyph, utf::Codepoint c, glm::vec2 pos, glm::vec2 scale) const;
		float advance_width(utf::Codepoint c, utf::Codepoint next_codepoint) const;
	};

	struct TextElement
	{
		Font font;
		utf::String text = "";
		glm::vec4 text_color = glm::vec4(1.0f);
		float adj_offset = 0.0f;
		glm::vec2 scale = glm::vec2(1.0f);
		BoundedFloat<0.0f, 1.0f> line_y_pivot = 0.0f;
		glm::vec2 jitter_offset = {};

		float line_height() const;

		static std::vector<TextElement> expand(const TextElement& element);
		static void expand(const TextElement& element, std::vector<TextElement>& to);

		void set_glyph(TextGlyph& glyph, utf::Codepoint c, glm::vec2 pos) const { font.set_glyph(glyph, c, pos, scale); }
		float advance_width(utf::Codepoint c, utf::Codepoint next_codepoint) const { return font.advance_width(c, next_codepoint) * scale.x; }
	};
}
