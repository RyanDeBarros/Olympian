#include "StorageMode.h"

namespace oly::detail
{
	std::ostream& operator<<(std::ostream& os, StorageMode mode)
	{
		os << "StorageMode(";

		switch (mode)
		{
		case StorageMode::Discard:
			os << "Discard";
			break;

		case StorageMode::Keep:
			os << "Keep";
			break;

		default:
			os << "unknown";
			break;
		}

		return os << ")";
	}
}
