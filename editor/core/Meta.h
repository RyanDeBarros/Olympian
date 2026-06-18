#pragma once

#include <type_traits>

namespace oly::editor
{
	template<typename T>
	concept Enum = std::is_enum_v<T>;
}
