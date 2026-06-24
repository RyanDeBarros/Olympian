#include "MemoryUnit.h"

namespace oly::editor
{
	size_t MemorySize(size_t count, MemoryUnit unit)
	{
		switch (unit)
		{
		case MemoryUnit::B:
			return count;

		case MemoryUnit::KB:
			return count * 1000ULL;

		case MemoryUnit::KiB:
			return count * 1024ULL;

		case MemoryUnit::MB:
			return count * 1000ULL * 1000ULL;

		case MemoryUnit::MiB:
			return count * 1024ULL * 1024ULL;

		case MemoryUnit::GB:
			return count * 1000ULL * 1000ULL * 1000ULL;

		case MemoryUnit::GiB:
			return count * 1024ULL * 1024ULL * 1024ULL;

		default:
			return 0;
		}
	}
}
