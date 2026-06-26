#pragma once

#include <ostream>

namespace oly::editor
{
	enum class MemoryUnit
	{
		B,
		KB,
		KiB,
		MB,
		MiB,
		GB,
		GiB
	};

	extern size_t MemorySize(size_t count, MemoryUnit unit);
	extern std::ostream& operator<<(std::ostream& os, MemoryUnit unit);
}
