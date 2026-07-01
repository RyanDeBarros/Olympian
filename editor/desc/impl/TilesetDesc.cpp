#include "TilesetDesc.h"

#include "definitions/Keys.h"

namespace oly::editor
{
	TilesetAssignmentDesc::TilesetAssignmentDesc() :
		texture("", detail::Key::TextureFile, "Texture"),
		texture_index(0, detail::Key::TextureIndex, "Texture Slot"),
		uvs({}, detail::Key::UVvec4, "Texture UVs"),
		reflection(detail::TILE_REFLECTION_BITSET_DEFAULT, detail::Key::Reflection, "Reflection", detail::TILE_REFLECTION_BITSET_VALUES, detail::TILE_REFLECTION_BITSET_NAMES, true),
		rotation(detail::TileRotation::None, detail::Key::Rotation, "Rotation")
	{
	}

	TilesetAssignmentMapDesc::TilesetAssignmentMapDesc()
	{
	}

	const detail::Key TilesetDesc::assignments_key = detail::Key::AssignmentArray;

	TilesetDesc::TilesetDesc() :
		storage(detail::StorageMode::Keep, detail::Key::Storage, "Storage")
	{
	}
}
