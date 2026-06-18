#include "FontFamily.h"

#include "core/algorithms/STLUtils.h"

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
		else if (family->supports(style & FontStyle::BOLD))
			return family->get(style & FontStyle::BOLD);
		else if (family->supports(FontStyle::REGULAR))
			return family->get(FontStyle::REGULAR);
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
