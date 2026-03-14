#pragma once

#include "core/types/SmartReference.h"
#include "core/util/ResourcePath.h"

namespace oly::rendering
{
	class TileSet;
}

namespace oly::context
{
	extern SmartReference<rendering::TileSet> load_tileset(const ResourcePath& file);
	extern void free_tileset(const ResourcePath& file);
}
