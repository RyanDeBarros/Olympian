#include "UID.h"

namespace oly::editor
{
	const char* StringID(UID uid)
	{
		switch (uid)
		{
#define SWITCH_CASE(E) case UID::E: return #E;
			UID_GENERATOR(SWITCH_CASE)
#undef SWITCH_CASE
		default:
			return "";
		}
	}
}
