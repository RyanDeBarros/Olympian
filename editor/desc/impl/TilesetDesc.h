#pragma once

#include "desc/Fields.h"

#include "definitions/enums/StorageMode.h"
#include "definitions/enums/TilesetConfiguration.h"

namespace oly::editor
{
#define TILESET_ASSIGNMENT_GENERATOR(M) \
		M((StringField), texture) \
		M((IntField<imp::potential<int>(0), imp::nullpotential>), texture_index) \
		M((UVRectField), uvs) \
		M((BitsetField<detail::TileReflection, detail::TILE_REFLECTION_BITSET_COUNT>), reflection) \
		M((EnumField<detail::TileRotation>), rotation)

	struct TilesetAssignmentDesc
	{
		IMTK_DESCRIPTOR_BODY(TilesetAssignmentDesc, TILESET_ASSIGNMENT_GENERATOR);

		TilesetAssignmentDesc(imtk::datapath_link link = {});
	};

#define TILESET_ASSIGNMENT_MAP_GENERATOR(M) \
		M((imtk::desc::map<detail::TileConfig, TilesetAssignmentDesc>), map)

	struct TilesetAssignmentMapDesc
	{
		IMTK_DESCRIPTOR_BODY(TilesetAssignmentMapDesc, TILESET_ASSIGNMENT_MAP_GENERATOR);

		TilesetAssignmentMapDesc(imtk::datapath_link link = {});
	};

#define TILESET_PARTIAL_GENERATOR(M) \
		M((EnumField<detail::StorageMode>), storage)

#define TILESET_GENERATOR(M) \
		TILESET_PARTIAL_GENERATOR(M) \
		M((TilesetAssignmentMapDesc), assignments)

	struct TilesetDesc
	{
		IMTK_DESCRIPTOR_BODY(TilesetDesc, TILESET_GENERATOR);

		static const detail::Key assignments_key;

		TilesetDesc(imtk::datapath_link link = {});
	};
}
