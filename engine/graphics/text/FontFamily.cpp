#include "FontFamily.h"

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
			throw Error(ErrorCode::INVALID_ID);
	}

	FontFamily::FontRef FontSelection::get() const
	{
		return family->get(style);
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
