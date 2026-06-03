#pragma once

#include "graphics/text/Font.h"
#include "graphics/text/RasterFont.h"
#include "core/types/Variant.h"

#include <optional>

namespace oly::rendering
{
	struct FontStyle
	{
		enum Mode : unsigned int
		{
			Regular = 0,
			Bold = 0b1,
			Italic = 0b10,
			BoldItalic = 0b11
		};

	private:
		unsigned int v = 0;

	public:
		constexpr FontStyle(Mode v) : v(v & BoldItalic) {}

		constexpr bool operator==(FontStyle other) const { return v == other.v; }
		constexpr operator unsigned int() const { return v; }

		constexpr FontStyle operator~() const { return Mode(~v & BoldItalic); }
		constexpr FontStyle operator|(FontStyle other) const { return Mode(v | other.v); }
		constexpr FontStyle operator|(Mode other) const { return Mode(v | other); }
		constexpr FontStyle operator&(FontStyle other) const { return Mode(v & other.v); }
		constexpr FontStyle operator&(Mode other) const { return Mode(v & other); }
		constexpr FontStyle& operator|=(FontStyle other) { return *this = (*this | other); }
		constexpr FontStyle& operator|=(Mode other) { return *this = (*this | other); }
		constexpr FontStyle& operator&=(FontStyle other) { return *this = (*this & other); }
		constexpr FontStyle& operator&=(Mode other) { return *this = (*this & other); }
	};
}

template<>
struct std::hash<oly::rendering::FontStyle>
{
	size_t operator()(const oly::rendering::FontStyle& fs) const { return std::hash<unsigned int>{}(fs); }
};

namespace oly::rendering
{
	struct FontFamily
	{
		using FontRef = Variant<FontAtlasRef, RasterFontRef>;

		std::unordered_map<FontStyle, FontRef> styles;

		bool supports(FontStyle key) const;
		FontRef get(FontStyle key) const;
	};

	typedef SmartReference<FontFamily> FontFamilyRef;

	struct FontSelection
	{
		FontFamilyRef family;
		FontStyle style = FontStyle::Regular;

		FontFamily::FontRef get() const;
		void set_font(const FontFamily::FontRef& font);
		bool style_exists() const;
	};
}
