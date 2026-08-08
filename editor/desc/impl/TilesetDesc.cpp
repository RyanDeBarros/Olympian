#include "TilesetDesc.h"

#include "definitions/Keys.h"

namespace oly::editor
{
	TilesetAssignmentDesc::TilesetAssignmentDesc(DataPathLink link) :
		link(std::move(link)),
		texture(DATA_PATH_SUBLINK(subpaths.texture), "", detail::Key::TextureFile, "Texture"),
		texture_index(DATA_PATH_SUBLINK(subpaths.texture_index), 0, detail::Key::TextureIndex, "Texture Slot"),
		uvs(DATA_PATH_SUBLINK(subpaths.uvs), {}, detail::Key::UVvec4, "Texture UVs"),
		reflection(DATA_PATH_SUBLINK(subpaths.reflection), detail::TILE_REFLECTION_BITSET_DEFAULT, detail::Key::Reflection, "Reflection",
			detail::TILE_REFLECTION_BITSET_VALUES, detail::TILE_REFLECTION_BITSET_NAMES, true),
		rotation(DATA_PATH_SUBLINK(subpaths.rotation), detail::TileRotation::None, detail::Key::Rotation, "Rotation")
	{
	}

	TilesetAssignmentMapDesc::TilesetAssignmentMapDesc(DataPathLink link) :
		link(std::move(link)),
		map(DATA_PATH_SUBLINK(subpaths.map))
	{
	}

	const detail::Key TilesetDesc::assignments_key = detail::Key::AssignmentArray;

	TilesetDesc::TilesetDesc(DataPathLink link) :
		link(std::move(link)),
		storage(DATA_PATH_SUBLINK(subpaths.storage), detail::StorageMode::Keep, detail::Key::Storage, "Storage"),
		assignments(DATA_PATH_SUBLINK(subpaths.assignments))
	{
	}
}
