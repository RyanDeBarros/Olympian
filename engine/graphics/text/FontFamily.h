#pragma once

#include "graphics/text/Font.h"
#include "graphics/text/RasterFont.h"

#include "definitions/enums/FontStyle.h"

#include <imp/variant.hpp>

#include <optional>

namespace oly::rendering
{
    class FontStyle
    {
        unsigned int v = 0;

    public:
        constexpr FontStyle(detail::FontStyleMode v)
            : v(v& detail::FontStyleMode::BoldItalic) // clamp to 0b11
        {
        }

        constexpr bool operator==(FontStyle o) const
        {
            return v == o.v;
        }

        constexpr operator unsigned int() const
        {
            return v;
        }

        constexpr FontStyle operator~() const
        {
            return detail::FontStyleMode(~v & detail::FontStyleMode::BoldItalic);
        }

        constexpr FontStyle operator|(FontStyle o) const
        {
            return detail::FontStyleMode(v | o.v);
        }

        constexpr FontStyle operator|(detail::FontStyleMode o) const
        {
            return detail::FontStyleMode(v | o);
        }

        constexpr FontStyle operator&(FontStyle o) const
        {
            return detail::FontStyleMode(v & o.v);
        }

        constexpr FontStyle operator&(detail::FontStyleMode o) const
        {
            return detail::FontStyleMode(v & o);
        }

        constexpr FontStyle& operator|=(FontStyle o)
        {
            v |= o.v;
            return *this;
        }

        constexpr FontStyle& operator|=(detail::FontStyleMode o)
        {
            v |= o;
            return *this;
        }

        constexpr FontStyle& operator&=(FontStyle o)
        {
            v &= o.v;
            return *this;
        }

        constexpr FontStyle& operator&=(detail::FontStyleMode o)
        {
            v &= o;
            return *this;
        }

        static const FontStyle kRegular;
        static const FontStyle kBold;
        static const FontStyle kItalic;
        static const FontStyle kBoldItalic;
    };

    inline const FontStyle FontStyle::kRegular = FontStyle(detail::FontStyleMode::Regular);
    inline const FontStyle FontStyle::kBold = FontStyle(detail::FontStyleMode::Bold);
    inline const FontStyle FontStyle::kItalic = FontStyle(detail::FontStyleMode::Italic);
    inline const FontStyle FontStyle::kBoldItalic = FontStyle(detail::FontStyleMode::BoldItalic);
}

template<>
struct std::hash<oly::rendering::FontStyle>
{
    size_t operator()(const oly::rendering::FontStyle& fs) const;
};

namespace oly::rendering
{
    struct FontFamily
    {
        using FontRef = imp::variant<FontAtlasRef, RasterFontRef>;

        std::unordered_map<FontStyle, FontRef> styles;

        bool supports(FontStyle key) const;
        FontRef get(FontStyle key) const;
    };

    using FontFamilyRef = SmartReference<FontFamily>;

    struct FontSelection
    {
        FontFamilyRef family;
        FontStyle style = FontStyle::kRegular;

        FontFamily::FontRef get() const;
        void set_font(const FontFamily::FontRef& font);
        bool style_exists() const;
    };
}
