#pragma once

#include "core/base/SmartReference.h"

#include "assets/ResourcePath.h"

namespace oly::rendering
{
	class TileSet;
}

namespace oly::context
{
	extern SmartReference<rendering::TileSet> load_tileset(const detail::ResourcePath& file);
	extern void free_tileset(const detail::ResourcePath& file);
}
