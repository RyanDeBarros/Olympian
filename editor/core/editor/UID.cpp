#include "UID.h"

namespace oly::editor
{
	const char* StringID(UID uid)
	{
		switch (uid)
		{
		case UID::DynamicRowReorder:
			return "DYNAMIC_ROW_REORDER";
		default:
			return "";
		}
	}
}
