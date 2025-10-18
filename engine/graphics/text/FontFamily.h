#pragma once

#include "graphics/text/Font.h"
#include "graphics/text/RasterFont.h"

#include <variant>

namespace oly::rendering
{
	struct FontStyle
	{
	private:
		enum class Defaults : int
		{
			REGULAR,
			BOLD,
			ITALIC,
			BOLD_ITALIC
			// TODO v5 other styles?
		};

	public:
		int v = 0;

		constexpr FontStyle() = default;
		constexpr FontStyle(int v) : v(v) {}

		bool operator==(const FontStyle& other) const { return v == other.v; }

	private:
		constexpr FontStyle(Defaults v) : v((int)v) {}

	public:
		static constexpr FontStyle REGULAR() { return FontStyle(Defaults::REGULAR); }
		static constexpr FontStyle BOLD() { return FontStyle(Defaults::BOLD); }
		static constexpr FontStyle ITALIC() { return FontStyle(Defaults::ITALIC); }
		static constexpr FontStyle BOLD_ITALIC() { return FontStyle(Defaults::BOLD_ITALIC); }
	};
}

template<>
struct std::hash<oly::rendering::FontStyle>
{
	size_t operator()(const oly::rendering::FontStyle& fs) const { return std::hash<int>{}(fs.v); }
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

	// TODO v5 FontSelection registry
	struct FontSelection
	{
		FontFamilyRef family;
		FontStyle style = FontStyle::REGULAR();

		FontFamily::FontRef get() const;
		void set_font(const FontFamily::FontRef& font);
		bool style_exists() const;
	};
}
