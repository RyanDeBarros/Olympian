#pragma once

#include "core/types/SmartReference.h"
#include "detail/assets/ResourcePath.h"

namespace oly::rendering
{
	class TileSet;
}

namespace oly::context
{
	extern SmartReference<rendering::TileSet> load_tileset(const detail::ResourcePath& file);
	extern void free_tileset(const detail::ResourcePath& file);
}
