#pragma once

#include "gui/PropertyClipboard.h"

namespace oly::editor::prop
{
	template<typename T>
	RawPropertyPayload MakePropertyPayload(const T& value);

	template<typename T>
	bool CanParsePropertyPayload(const RawPropertyPayload& payload);

	template<typename T>
	bool TryParsePropertyPayload(const RawPropertyPayload& payload, T& value);
}
