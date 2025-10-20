#pragma once

#include "graphics/text/Font.h"
#include "graphics/text/RasterFont.h"
#include "core/types/Variant.h"

#include <variant>
#include <optional>

namespace oly::rendering
{
	struct FontStyle
	{
	private:
		enum Defaults : unsigned int
		{
			D_REGULAR = 0,
			D_BOLD = 0b1,
			D_ITALIC = 0b10,
			D_MAX = 0b11
		};

		unsigned int v = 0;

	public:
		constexpr FontStyle(unsigned int v) : v(v & Defaults::D_MAX) {}

		constexpr bool operator==(FontStyle other) const { return v == other.v; }
		constexpr operator unsigned int() const { return v; }

		constexpr FontStyle operator~() const { return FontStyle(~v & Defaults::D_MAX); }
		constexpr FontStyle operator|(FontStyle other) const { return FontStyle(v | other.v); }
		constexpr FontStyle operator&(FontStyle other) const { return FontStyle(v & other.v); }
		constexpr FontStyle& operator|=(FontStyle other) { return *this = (*this | other); }
		constexpr FontStyle& operator&=(FontStyle other) { return *this = (*this & other); }

		static constexpr FontStyle REGULAR() { return FontStyle(Defaults::D_REGULAR); }
		static constexpr FontStyle BOLD() { return FontStyle(Defaults::D_BOLD); }
		static constexpr FontStyle ITALIC() { return FontStyle(Defaults::D_ITALIC); }
		static constexpr FontStyle BOLD_ITALIC() { return FontStyle(Defaults::D_BOLD | Defaults::D_ITALIC); }

		static std::optional<FontStyle> from_string(const std::string& str);
		static std::optional<FontStyle> from_string(std::string&& str);
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
		using FontRef = std::variant<FontAtlasRef, RasterFontRef>;

		std::unordered_map<FontStyle, FontRef> styles;

		bool supports(FontStyle key) const;
		FontRef get(FontStyle key) const;
	};

	typedef SmartReference<FontFamily> FontFamilyRef;

	struct FontSelection
	{
		FontFamilyRef family;
		FontStyle style = FontStyle::REGULAR();

		FontFamily::FontRef get() const;
		void set_font(const FontFamily::FontRef& font);
		bool style_exists() const;
	};
}
