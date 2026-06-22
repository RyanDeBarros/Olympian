#pragma once

#include "gui/PropertyClipboard.h"

#include <optional>

namespace oly::editor::prop
{
	template<typename T>
	RawPropertyPayload MakePropertyPayload(const T& value);

	template<typename T>
	std::optional<T> ParsePropertyPayload(const RawPropertyPayload& payload);
}
