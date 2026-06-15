#include "TilesetConfiguration.h"

namespace oly::detail
{
	TileTransformation TILE_TRANSFORMATION_DEFAULT = TileTransformation::None;
	
	TileTransformation TILE_TRANSFORMATION_VALUES[TILE_TRANSFORMATION_COUNT] = {
		TileTransformation::ReflectX,
		TileTransformation::ReflectY,
		TileTransformation::Rotate90,
		TileTransformation::Rotate180,
		TileTransformation::Rotate270
	};

	const char* TILE_TRANSFORMATION_NAMES[TILE_TRANSFORMATION_COUNT] = {
		"Reflect (X)",
		"Reflect (Y)",
		"Rotate 90",
		"Rotate 180",
		"Rotate 270",
	};
}
