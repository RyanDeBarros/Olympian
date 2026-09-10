#pragma once

#include "core/Types.h"
#include "bindings/Widgets.h"
#include "bindings/Serializer.h"

#include "bindings/FieldBase.h"

namespace oly::editor
{
	using RectField = PrimitiveField<Rect>;
	using UVRectField = PrimitiveField<UVRect>;
	using TopSidePaddingField = PrimitiveField<TopSidePadding>;
}
