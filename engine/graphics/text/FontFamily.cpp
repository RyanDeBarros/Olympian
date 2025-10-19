#include "FontFamily.h"

#include "core/algorithms/STLUtils.h"

namespace oly::rendering
{
	std::optional<FontStyle> FontStyle::from_string(const std::string& str)
	{
		return from_string(dupl(str));
	}

	std::optional<FontStyle> FontStyle::from_string(std::string&& str)
	{
		algo::to_lower(str);
		if (str == "regular")
			return rendering::FontStyle::REGULAR();
		else if (str == "bold")
			return rendering::FontStyle::BOLD();
		else if (str == "italic")
			return rendering::FontStyle::ITALIC();
		else if (str == "bolditalic" || str == "bold italic" || str == "bold_italic")
			return rendering::FontStyle::BOLD_ITALIC();
		else
			return std::nullopt;
	}

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
	
	bool FontSelection::try_apply_style(FontStyle s)
	{
		switch (s)
		{
		case FontStyle::BOLD():
			if (style == FontStyle::ITALIC() && family->supports(FontStyle::BOLD_ITALIC()))
			{
				style = FontStyle::BOLD_ITALIC();
				return true;
			}
			break;
		case FontStyle::ITALIC():
			if (style == FontStyle::BOLD() && family->supports(FontStyle::BOLD_ITALIC()))
			{
				style = FontStyle::BOLD_ITALIC();
				return true;
			}
			break;
		}

		if (family->supports(s))
		{
			style = s;
			return true;
		}
		else
			return false;
	}

	bool FontSelection::try_unapply_style(FontStyle s)
	{
		switch (s)
		{
		case FontStyle::BOLD():
			if (style == FontStyle::BOLD_ITALIC() && family->supports(FontStyle::ITALIC()))
			{
				style = FontStyle::ITALIC();
				return true;
			}
			break;
		case FontStyle::ITALIC():
			if (style == FontStyle::BOLD_ITALIC() && family->supports(FontStyle::BOLD()))
			{
				style = FontStyle::BOLD();
				return true;
			}
			break;
		}

		if (style == s)
		{
			style = FontStyle::REGULAR();
			return true;
		}
		else
			return false;
	}
}
