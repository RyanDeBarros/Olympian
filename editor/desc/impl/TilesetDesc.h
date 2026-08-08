#pragma once

#include "desc/Fields.h"
#include "desc/Descriptors.h"

#include "definitions/enums/StorageMode.h"
#include "definitions/enums/TilesetConfiguration.h"

namespace oly::editor
{
#define TILESET_ASSIGNMENT_GENERATOR(M) \
		M(texture) \
		M(texture_index) \
		M(uvs) \
		M(reflection) \
		M(rotation)

	struct TilesetAssignmentDesc
	{
		DESCRIPTOR_BODY(TilesetAssignmentDesc, TILESET_ASSIGNMENT_GENERATOR);

		StringField texture;
		IntField<MakeOpt(0), MakeOpt<int>()> texture_index;
		UVRectField uvs;
		BitsetField<detail::TileReflection, detail::TILE_REFLECTION_BITSET_COUNT> reflection;
		EnumField<detail::TileRotation> rotation;

		TilesetAssignmentDesc(DataPathLink link = {});
	};

#define TILESET_ASSIGNMENT_MAP_GENERATOR(M) \
		M(map)

	struct TilesetAssignmentMapDesc
	{
		DESCRIPTOR_BODY(TilesetAssignmentMapDesc, TILESET_ASSIGNMENT_MAP_GENERATOR);

		MapDesc<detail::TileConfig, TilesetAssignmentDesc> map;

		TilesetAssignmentMapDesc(DataPathLink link = {});
	};

#define TILESET_PARTIAL_GENERATOR(M) \
		M(storage)

#define TILESET_GENERATOR(M) \
		TILESET_PARTIAL_GENERATOR(M) \
		M(assignments)

	struct TilesetDesc
	{
		DESCRIPTOR_BODY(TilesetDesc, TILESET_GENERATOR);

		EnumField<detail::StorageMode> storage;
		TilesetAssignmentMapDesc assignments;
		static const detail::Key assignments_key;

		TilesetDesc(DataPathLink link = {});
	};
}
