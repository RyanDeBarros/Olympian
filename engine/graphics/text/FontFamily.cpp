#include "FontFamily.h"

#include "core/algorithms/STLUtils.h"

size_t std::hash<oly::rendering::FontStyle>::operator()(const oly::rendering::FontStyle& fs) const
{
    return std::hash<unsigned int>{}(fs);
}

namespace oly::rendering
{
    bool FontFamily::supports(FontStyle key) const
    {
        return styles.count(key);
    }

    FontFamily::FontRef FontFamily::get(FontStyle key) const
    {
        auto it = styles.find(key);
        if (it != styles.end())
            return it->second;
        else
            throw Error(ErrorCode::InvalidID);
    }

    FontFamily::FontRef FontSelection::get() const
    {
        if (style_exists())
            return family->get(style);
        else if (family->supports(style & FontStyle::kBold))
            return family->get(style & FontStyle::kBold);
        else if (family->supports(FontStyle::kRegular))
            return family->get(FontStyle::kRegular);
        else
            return FontAtlasRef(nullptr);
    }

    void FontSelection::set_font(const FontFamily::FontRef& font)
    {
        family->styles[style] = font;
    }

    bool FontSelection::style_exists() const
    {
        return family->supports(style);
    }
}
