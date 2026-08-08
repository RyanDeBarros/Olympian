#include "TilesetDesc.h"

#include "definitions/Keys.h"

namespace oly::editor
{
	TilesetAssignmentDesc::TilesetAssignmentDesc(imtk::datapath_link link) :
		link(std::move(link)),
		texture(IMTK_DATAPATH_SUBLINK(subpaths.texture), "", detail::Key::TextureFile, "Texture"),
		texture_index(IMTK_DATAPATH_SUBLINK(subpaths.texture_index), 0, detail::Key::TextureIndex, "Texture Slot"),
		uvs(IMTK_DATAPATH_SUBLINK(subpaths.uvs), {}, detail::Key::UVvec4, "Texture UVs"),
		reflection(IMTK_DATAPATH_SUBLINK(subpaths.reflection), detail::TILE_REFLECTION_BITSET_DEFAULT, detail::Key::Reflection, "Reflection",
			detail::TILE_REFLECTION_BITSET_VALUES, detail::TILE_REFLECTION_BITSET_NAMES, true),
		rotation(IMTK_DATAPATH_SUBLINK(subpaths.rotation), detail::TileRotation::None, detail::Key::Rotation, "Rotation")
	{
	}

	TilesetAssignmentMapDesc::TilesetAssignmentMapDesc(imtk::datapath_link link) :
		link(std::move(link)),
		map(IMTK_DATAPATH_SUBLINK(subpaths.map))
	{
	}

	const detail::Key TilesetDesc::assignments_key = detail::Key::AssignmentArray;

	TilesetDesc::TilesetDesc(imtk::datapath_link link) :
		link(std::move(link)),
		storage(IMTK_DATAPATH_SUBLINK(subpaths.storage), detail::StorageMode::Keep, detail::Key::Storage, "Storage"),
		assignments(IMTK_DATAPATH_SUBLINK(subpaths.assignments))
	{
	}
}
