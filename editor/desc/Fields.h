#pragma once

#include "core/Types.h"
#include "gui/Widgets.h"
#include "desc/Serializer.h"

#include "desc/FieldBase.h"

namespace oly::editor
{
	using RectField = PrimitiveField<Rect>;
	using UVRectField = PrimitiveField<UVRect>;
	using TopSidePaddingField = PrimitiveField<TopSidePadding>;
}
