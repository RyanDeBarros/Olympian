#pragma once

namespace oly::editor
{
#define UID_GENERATOR(M) \
	M(DynamicRowReorder) \
	M(PathDrag)

#define ENUM_ENTRY(E) E,

	enum class UID
	{
		UID_GENERATOR(ENUM_ENTRY)
	};

#undef ENUM_ENTRY

	extern const char* StringID(UID uid);
}
