#pragma once

#include "core/base/Parameters.h"
#include "core/util/UTF.h"

#include "graphics/text/Font.h"
#include "graphics/text/RasterFont.h"

namespace oly::rendering
{
	// TODO v5 something more efficient than variant?
	// TODO v5 Add FontStyle: a new struct that holds a FontFamily, and style index into the family. FontFamily is a mapping of styles to font refs (styles being regular, bold, italic, etc.). This can then easily be used for the separators (defined below).
	using TextElementFont = std::variant<FontAtlasRef, RasterFontRef>;

	namespace internal
	{
		namespace TextElementFontIndex
		{
			enum
			{
				ATLAS = 0,
				RASTER = 1
			};
		};
	}

	struct TextElement
	{
		TextElementFont font;
		utf::String text = "";
		glm::vec4 text_color = glm::vec4(1.0f);
		float adj_offset = 0.0f;
		glm::vec2 scale = glm::vec2(1.0f);
		BoundedFloat<0.0f, 1.0f> line_y_pivot = 0.0f;
		glm::vec2 jitter_offset = {};

		float line_height() const;

		static std::vector<TextElement> expand(const TextElement& element);
	};

	class TextGlyph;

	namespace internal
	{
		extern bool font_equals(const TextElement& element, const FontAtlasRef& font);
		extern bool font_equals(const TextElement& element, const RasterFontRef& font);
		extern bool has_same_font_face(const TextElement& a, const TextElement& b);
		extern bool support(const TextElement& element, utf::Codepoint c);
		extern void set_glyph(TextGlyph& glyph, const TextElement& element, utf::Codepoint c, glm::vec2 pos);
		extern float advance_width(const TextElement& element, utf::Codepoint c, utf::Codepoint next_codepoint);
	}
}
