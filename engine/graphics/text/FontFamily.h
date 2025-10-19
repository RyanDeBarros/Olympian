#pragma once

#include "graphics/text/Font.h"
#include "graphics/text/RasterFont.h"

#include <variant>
#include <optional>

namespace oly::rendering
{
	struct FontStyle
	{
	private:
		enum class Defaults : unsigned int
		{
			REGULAR,
			BOLD,
			ITALIC,
			BOLD_ITALIC
			// TODO v5 other styles?
		};

	public:
		unsigned int v = 0;

		constexpr FontStyle() = default;
		constexpr FontStyle(unsigned int v) : v(v) {}

		bool operator==(const FontStyle& other) const { return v == other.v; }
		operator unsigned int() const { return v; }

	private:
		constexpr FontStyle(Defaults v) : v((unsigned int)v) {}

	public:
		static constexpr FontStyle REGULAR() { return FontStyle(Defaults::REGULAR); }
		static constexpr FontStyle BOLD() { return FontStyle(Defaults::BOLD); }
		static constexpr FontStyle ITALIC() { return FontStyle(Defaults::ITALIC); }
		static constexpr FontStyle BOLD_ITALIC() { return FontStyle(Defaults::BOLD_ITALIC); }

		static std::optional<FontStyle> from_string(const std::string& str);
		static std::optional<FontStyle> from_string(std::string&& str);
	};
}

template<>
struct std::hash<oly::rendering::FontStyle>
{
	size_t operator()(const oly::rendering::FontStyle& fs) const { return std::hash<unsigned int>{}(fs.v); }
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
