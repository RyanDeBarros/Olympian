#pragma once

#include <ostream>

namespace oly::detail
{
	enum class StorageMode
	{
		Discard = 0,
		Keep
	};

	extern std::ostream& operator<<(std::ostream& os, StorageMode mode);
}
