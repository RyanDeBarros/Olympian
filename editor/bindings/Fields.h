#pragma once

#include "core/Types.h"
#include "bindings/Widgets.h"
#include "bindings/Serializer.h"

namespace imtk::field
{
	using rect_fld = primitive_fld<oly::editor::Rect>;
	using uv_rect_fld = primitive_fld<oly::editor::UVRect>;
	using top_side_padding_fld = primitive_fld<oly::editor::TopSidePadding>;
}
