#pragma once

#include "graphics/text/Font.h"
#include "graphics/text/RasterFont.h"
#include "core/types/Variant.h"

#include "definitions/enums/FontStyle.h"

#include <optional>

namespace oly::rendering
{
	class FontStyle
	{
		unsigned int v = 0;

	public:
		constexpr FontStyle(detail::FontStyleMode v) : v(v & detail::FontStyleMode::BoldItalic) {}

		constexpr bool operator==(FontStyle other) const { return v == other.v; }
		constexpr operator unsigned int() const { return v; }

		constexpr FontStyle operator~() const { return detail::FontStyleMode(~v & detail::FontStyleMode::BoldItalic); }
		constexpr FontStyle operator|(FontStyle other) const { return detail::FontStyleMode(v | other.v); }
		constexpr FontStyle operator|(detail::FontStyleMode other) const { return detail::FontStyleMode(v | other); }
		constexpr FontStyle operator&(FontStyle other) const { return detail::FontStyleMode(v & other.v); }
		constexpr FontStyle operator&(detail::FontStyleMode other) const { return detail::FontStyleMode(v & other); }
		constexpr FontStyle& operator|=(FontStyle other) { return *this = (*this | other); }
		constexpr FontStyle& operator|=(detail::FontStyleMode other) { return *this = (*this | other); }
		constexpr FontStyle& operator&=(FontStyle other) { return *this = (*this & other); }
		constexpr FontStyle& operator&=(detail::FontStyleMode other) { return *this = (*this & other); }

		static const FontStyle REGULAR;
		static const FontStyle BOLD;
		static const FontStyle ITALIC;
		static const FontStyle BOLD_ITALIC;
	};

	inline const FontStyle FontStyle::REGULAR = FontStyle(detail::FontStyleMode::Regular);
	inline const FontStyle FontStyle::BOLD = FontStyle(detail::FontStyleMode::Bold);
	inline const FontStyle FontStyle::ITALIC = FontStyle(detail::FontStyleMode::Italic);
	inline const FontStyle FontStyle::BOLD_ITALIC = FontStyle(detail::FontStyleMode::BoldItalic);
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
		FontStyle style = FontStyle::REGULAR;

		FontFamily::FontRef get() const;
		void set_font(const FontFamily::FontRef& font);
		bool style_exists() const;
	};
}
