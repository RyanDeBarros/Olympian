#pragma once

#include "assets/ResourcePath.h"

namespace oly::editor::fio
{
	struct Trashcan
	{
		static bool Delete(const detail::ResourcePath& resource);
		static bool Restore(const detail::ResourcePath& resource);
	};
}
