#pragma once

namespace oly::detail
{
	enum FontStyleMode : unsigned int
	{
		Regular = 0,
		Bold = 1 << 0,
		Italic = 1 << 1,
		BoldItalic = Bold | Italic
	};
}
