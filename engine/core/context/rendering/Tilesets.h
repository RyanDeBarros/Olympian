#pragma once

#include "core/types/SmartReference.h"

namespace oly::rendering
{
	class TileSet;
}

namespace oly::reg
{
	class TileSetRegistry;
}

namespace oly::context
{
	namespace internal
	{
		extern void terminate_tilesets();
	}

	extern reg::TileSetRegistry& tileset_registry();
	extern SmartReference<rendering::TileSet> load_tileset(const std::string& file);
}
