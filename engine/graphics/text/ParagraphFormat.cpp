#include "ParagraphFormat.h"

namespace oly::rendering
{
	bool ParagraphFormat::can_fit_on_line(TypesetData t, float dx) const
	{
		return text_wrap <= 0.0f || t.x + dx <= text_wrap;
	}

	bool ParagraphFormat::can_fit_vertically(TypesetData t, float dy) const
	{
		return max_height <= 0.0f || -t.y + dy <= max_height;
	}
}
