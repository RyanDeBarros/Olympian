#include "Tilesets.h"

#include "registries/graphics/sprites/TileSetRegistry.h"

namespace oly::context
{
	namespace internal
	{
		reg::TileSetRegistry tileset_registry;
	}

	void internal::terminate_tilesets()
	{
		internal::tileset_registry.clear();
	}

	reg::TileSetRegistry& tileset_registry()
	{
		return internal::tileset_registry;
	}

	rendering::TileSetRef load_tileset(const std::string& file)
	{
		return internal::tileset_registry.load_tileset(file);
	}
}
